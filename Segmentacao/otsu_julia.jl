using Images, FileIO, Dates, Base.Threads, Statistics

# Diretórios de entrada e saída
const DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas"
const DIRETORIO_SAIDA = "/mnt/c/Users/bicha/Documents/Saida_otsu"

#----------------------------------------------------------
# Verifica se um arquivo é uma imagem suportada
#----------------------------------------------------------
function verificarImagem(nome_arquivo::String)
    extensoes_suportadas = [".png", ".jpg", ".jpeg", ".bmp", ".tiff"]
    any(endswith(nome_arquivo, ext) for ext in extensoes_suportadas)
end

#----------------------------------------------------------
# Lista arquivos de imagem no diretório
#----------------------------------------------------------
function listarArquivos(diretorio::String)
    if !isdir(diretorio)
        println("Erro: Diretório não encontrado -> $diretorio")
        return []
    end
    arquivos = readdir(diretorio, join=true)
    imagens = filter(verificarImagem, arquivos)

    if isempty(imagens)
        println("Nenhuma imagem foi encontrada em $diretorio")
    end
    return imagens
end

#----------------------------------------------------------
# Converte imagem para escala de cinza se necessário
#----------------------------------------------------------
function converterCinza(imagem)
    if eltype(imagem) <: Color3  # Se a imagem for RGB
        return Gray.(imagem)  # Converte para escala de cinza
    end
    return imagem
end

#----------------------------------------------------------
# Calcula o histograma em paralelo
#----------------------------------------------------------
function calcularhistograma(imagem)
    imagem = converterCinza(imagem)  # Garante que a imagem está em escala de cinza

    histograma_global = zeros(Int, 256)
    num_threads = nthreads()
    histograma_local = [zeros(Int, 256) for _ in 1:num_threads]

    pixels_img = vec(imagem)

    @threads for i in 1:length(pixels_img)
        id_thread = threadid()
        valor_pixel = Int(pixels_img[i] * 255) + 1
        histograma_local[id_thread][valor_pixel] += 1
    end

    for t in 1:num_threads
        for i in 1:256
            histograma_global[i] += histograma_local[t][i]
        end
    end

    return histograma_global
end

#----------------------------------------------------------
# Função para calcular o limiar de Otsu
#----------------------------------------------------------
function calcularLimiarOtsu(histograma, total_pixels)
    p = histograma ./ total_pixels
    wB = cumsum(p)
    wF = 1 .- wB
    mediaB = cumsum(p .* (0:255))
    mediaF = (sum(p .* (0:255)) .- mediaB) ./ wF

    variancia_max, melhor_limiar = 0.0, 0
    @threads for t in 1:255
        if wB[t] > 0 && wF[t] > 0
            variancia_entre_classes = wB[t] * wF[t] * (mediaB[t] - mediaF[t])^2
            if variancia_entre_classes > variancia_max
                variancia_max = variancia_entre_classes
                melhor_limiar = t
            end
        end
    end
    return melhor_limiar / 255
end

#----------------------------------------------------------
# Aplica a limiarização
#----------------------------------------------------------
function aplicarLimiarizacao(imagem, limiar)
    @threads for i in eachindex(imagem)
        imagem[i] = imagem[i] > limiar ? 1.0 : 0.0
    end
end

#----------------------------------------------------------
# Processa uma única imagem e retorna o tempo de execução
#----------------------------------------------------------
function processarImagem(caminho_entrada::String, caminho_saida::String)
    inicio = time()

    imagem = load(caminho_entrada) |> converterCinza  # Converte para escala de cinza

    # Aplica a técnica de Otsu corretamente
    histograma = calcularhistograma(imagem)
    limiar = calcularLimiarOtsu(histograma, length(imagem))
    aplicarLimiarizacao(imagem, limiar)

    save(caminho_saida, imagem)  # Salva a imagem segmentada

    tempo_execucao = (time() - inicio) * 1000
    println("Imagem $(basename(caminho_entrada)) processada em $(round(tempo_execucao, digits=4)) ms")
    return tempo_execucao
end

#----------------------------------------------------------
# Processa todas as imagens e exibe o tempo médio
#----------------------------------------------------------
function processarDiretorio(diretorio_entrada::String, diretorio_saida::String)
    isdir(diretorio_saida) || mkpath(diretorio_saida)
    imagens = listarArquivos(diretorio_entrada)

    if isempty(imagens)
        println("⚠️ Nenhuma imagem foi processada.")
        return
    end

    tempos_execucao = Float64[]

    for caminho_entrada in imagens
        caminho_saida = joinpath(diretorio_saida, basename(caminho_entrada))
        push!(tempos_execucao, processarImagem(caminho_entrada, caminho_saida))
    end

    if !isempty(tempos_execucao)
        tempo_medio = mean(tempos_execucao)
        println("\n⏳ Tempo médio por imagem: $(round(tempo_medio, digits=4)) ms")
    else
        println("\nNenhuma imagem foi processada.")
    end
end

#----------------------------------------------------------
# Função principal
#----------------------------------------------------------
function main()
    println("🚀 Iniciando processamento com $(nthreads()) threads...\n")
    processarDiretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA)
end

main()
