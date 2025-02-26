using Images, FileIO, Dates, Base.Threads, Statistics

# Diretórios de entrada e saída
const DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas"
const DIRETORIO_SAIDA = "/mnt/c/Users/bicha/Documents/Saida_otsu"

#----------------------------------------------------------
# Verifica se um arquivo é uma imagem suportada
#----------------------------------------------------------
function verifica_imagem(nome_arquivo::String)
    extensoes = [".png", ".jpg", ".jpeg", ".bmp", ".tiff"]
    any(endswith(nome_arquivo, ext) for ext in extensoes)
end

#----------------------------------------------------------
# Lista arquivos de imagem no diretório
#----------------------------------------------------------
function listar_arquivos(diretorio::String)
    if !isdir(diretorio)
        println("Erro: Diretório não encontrado -> $diretorio")
        return []
    end
    arquivos = readdir(diretorio, join=true)
    imagens = filter(verifica_imagem, arquivos)
    
    if isempty(imagens)
        println("Nenhuma imagem foi encontrada em $diretorio")
    end
    return imagens
end

#----------------------------------------------------------
# Converte imagem para escala de cinza se necessário
#----------------------------------------------------------
function converter_para_cinza(img)
    if eltype(img) <: Color3  # Se a imagem for RGB
        return Gray.(img)  # Converte para escala de cinza
    end
    return img
end

#----------------------------------------------------------
# Calcula o histograma em paralelo
#----------------------------------------------------------
function calcular_histograma_paralelo(img)
    img = converter_para_cinza(img)  # Garante que a imagem está em escala de cinza

    hist_global = zeros(Int, 256)
    num_threads = nthreads()
    hist_local = [zeros(Int, 256) for _ in 1:num_threads]

    img_vec = vec(img)

    @threads for i in 1:length(img_vec)
        tid = threadid()
        pixel_val = Int(img_vec[i] * 255) + 1
        hist_local[tid][pixel_val] += 1
    end

    for t in 1:num_threads
        for i in 1:256
            hist_global[i] += hist_local[t][i]
        end
    end

    return hist_global
end

#----------------------------------------------------------
# Função para calcular o limiar de Otsu
#----------------------------------------------------------
function calcular_limiar_otsu(hist, total_pixels)
    p = hist ./ total_pixels
    wB = cumsum(p)
    wF = 1 .- wB
    meanB = cumsum(p .* (0:255))
    meanF = (sum(p .* (0:255)) .- meanB) ./ wF

    max_var, best_t = 0.0, 0
    @threads for t in 1:255
        if wB[t] > 0 && wF[t] > 0
            var_between = wB[t] * wF[t] * (meanB[t] - meanF[t])^2
            if var_between > max_var
                max_var = var_between
                best_t = t
            end
        end
    end
    return best_t / 255
end

#----------------------------------------------------------
# Aplica a limiarização
#----------------------------------------------------------
function aplicar_limiarizacao!(img, limiar)
    @threads for i in eachindex(img)
        img[i] = img[i] > limiar ? 1.0 : 0.0
    end
end

#----------------------------------------------------------
# Processa uma única imagem e retorna o tempo de execução
#----------------------------------------------------------
function processar_imagem(caminho_entrada::String, caminho_saida::String)
    inicio = time()

    img = load(caminho_entrada) |> converter_para_cinza  # Converte para escala de cinza

    # Aplica a técnica de Otsu corretamente
    hist = calcular_histograma_paralelo(img)
    limiar = calcular_limiar_otsu(hist, length(img))
    aplicar_limiarizacao!(img, limiar)

    save(caminho_saida, img)  # Salva a imagem segmentada

    tempo_execucao = (time() - inicio) * 1000
    println("Imagem $(basename(caminho_entrada)) processada em $(round(tempo_execucao, digits=2)) ms")
    return tempo_execucao
end

#----------------------------------------------------------
# Processa todas as imagens e exibe o tempo médio
#----------------------------------------------------------
function processar_diretorio(diretorio_entrada::String, diretorio_saida::String)
    isdir(diretorio_saida) || mkpath(diretorio_saida)
    imagens = listar_arquivos(diretorio_entrada)

    if isempty(imagens)
        println("⚠️ Nenhuma imagem foi processada.")
        return
    end

    tempos_execucao = Float64[]

    for caminho_entrada in imagens
        caminho_saida = joinpath(diretorio_saida, basename(caminho_entrada))
        push!(tempos_execucao, processar_imagem(caminho_entrada, caminho_saida))
    end

    if !isempty(tempos_execucao)
        media_tempo = mean(tempos_execucao)
        println("\n⏳ Tempo médio por imagem: $(round(media_tempo, digits=2)) ms")
    else
        println("\nNenhuma imagem foi processada.")
    end
end

#----------------------------------------------------------
# Função principal
#----------------------------------------------------------
function main()
    println("🚀 Iniciando processamento com $(nthreads()) threads...\n")
    processar_diretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA)
end

main()

