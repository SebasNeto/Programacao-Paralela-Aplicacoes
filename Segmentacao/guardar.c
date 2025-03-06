//otsu original em halide
#include "Halide.h"
#include "halide_image_io.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <vector>

namespace fs = std::filesystem;
using namespace Halide;

// Função para aplicar a limiarização de Otsu em uma imagem
Buffer<uint8_t> aplicar_otsu(const Buffer<uint8_t>& input) {
    // Se a imagem tiver 3 canais (RGB), converte para grayscale
    Buffer<uint8_t> gray;
    if (input.channels() == 3) {
        gray = Buffer<uint8_t>(input.width(), input.height());
        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                // Fórmula para conversão: 0.299*R + 0.587*G + 0.114*B
                float r = input(x, y, 0);
                float g = input(x, y, 1);
                float b = input(x, y, 2);
                gray(x, y) = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
            }
        }
    }
    else {
        // Se já for monocromática, usa diretamente
        gray = input;
    }

    // === Etapa 1: Calcular o histograma usando Halide ===
    Func histogram("histogram");
    Var i;
    // Inicializa o histograma com zero para 256 níveis
    histogram(i) = cast<int32_t>(0);
    RDom r(0, gray.width(), 0, gray.height());
    // Incrementa o histograma no índice correspondente ao valor do pixel
    histogram(gray(r.x, r.y)) += 1;

    // Realiza o cálculo do histograma (buffer de 256 elementos)
    Buffer<int32_t> hist = histogram.realize({ 256 });

    // === Etapa 2: Calcular o limiar de Otsu no host ===
    double total_pixels = gray.width() * gray.height();
    double sum_total = 0.0;
    for (int j = 0; j < 256; j++) {
        sum_total += j * hist(j);
    }

    double sumB = 0.0;
    double wB = 0.0;
    double max_variance = 0.0;
    int threshold = 0;

    for (int t = 0; t < 256; t++) {
        wB += hist(t);
        if (wB == 0)
            continue;

        double wF = total_pixels - wB;
        if (wF == 0)
            break;

        sumB += t * hist(t);
        double mB = sumB / wB;
        double mF = (sum_total - sumB) / wF;
        double variance = wB * wF * (mB - mF) * (mB - mF);

        if (variance > max_variance) {
            max_variance = variance;
            threshold = t;
        }
    }
    std::cout << "Limiar de Otsu calculado: " << threshold << std::endl;

    // === Etapa 3: Binarizar a imagem usando o limiar calculado ===
    Func binary("binary");
    Var x, y;
    binary(x, y) = select(gray(x, y) > threshold, cast<uint8_t>(255), cast<uint8_t>(0));

    // Agendamento para melhor desempenho
    binary.vectorize(x, 8).parallel(y);

    // Realiza a computação da imagem binarizada
    Buffer<uint8_t> output = binary.realize({ gray.width(), gray.height() });

    return output;
}


int main() {
    // Definir diretórios de entrada e saída
    const fs::path DIRETORIO_ENTRADA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC\\PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC\\Imagens_Selecionadas";
    const fs::path DIRETORIO_SAIDA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC 2024 - 2025\\Saida_Segmentacao";

    // Verificar se os diretórios existem
    if (!fs::exists(DIRETORIO_ENTRADA)) {
        std::cerr << "Diretório de entrada não encontrado!" << std::endl;
        return 1;
    }
    if (!fs::exists(DIRETORIO_SAIDA)) {
        if (!fs::create_directory(DIRETORIO_SAIDA)) {
            std::cerr << "Falha ao criar diretório de saída!" << std::endl;
            return 1;
        }
    }

    // Listar todas as imagens no diretório de entrada
    std::vector<fs::path> imagens;
    for (const auto& entry : fs::directory_iterator(DIRETORIO_ENTRADA)) {
        if (entry.is_regular_file() &&
            (entry.path().extension() == ".png" || entry.path().extension() == ".jpg")) {
            imagens.push_back(entry.path());
        }
    }

    if (imagens.empty()) {
        std::cerr << "Nenhuma imagem encontrada no diretório de entrada!" << std::endl;
        return 1;
    }

    // Processar cada imagem e medir o tempo de execução
    double tempo_total = 0.0;
    for (const auto& caminho_imagem : imagens) {
        // Carregar a imagem
        Buffer<uint8_t> input;
        try {
            input = Tools::load_image(caminho_imagem.string());
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao carregar a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        if (!input.defined()) {
            std::cerr << "Erro: Buffer da imagem " << caminho_imagem.filename()
                << " não foi carregado corretamente." << std::endl;
            continue;
        }

        // --- Execução de aquecimento (primeira execução descartada) ---
        try {
            Buffer<uint8_t> dummy = aplicar_otsu(input);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro na execução de aquecimento para a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        // Medir o tempo da segunda execução
        auto inicio = std::chrono::high_resolution_clock::now();

        Buffer<uint8_t> output;
        try {
            output = aplicar_otsu(input);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao processar a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        auto fim = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = fim - inicio;
        tempo_total += duracao.count();

        // Salvar a imagem binarizada
        fs::path caminho_saida = DIRETORIO_SAIDA / caminho_imagem.filename();
        try {
            Tools::save_image(output, caminho_saida.string());
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao salvar a imagem " << caminho_saida.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        std::cout << "Processada: " << caminho_imagem.filename()
            << " | Tempo: " << duracao.count() << "s" << std::endl;
    }

    // Calcular a média do tempo de execução por imagem (considerando somente execuções aquecidas)
    if (!imagens.empty()) {
        double tempo_medio = tempo_total / imagens.size();
        std::cout << "Tempo médio de execução por imagem: "
            << tempo_medio << "s" << std::endl;
    }
    else {
        std::cerr << "Nenhuma imagem foi processada com sucesso." << std::endl;
    }

    return 0;
}

//otsu original em julia
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
    println("Imagem $(basename(caminho_entrada)) processada em $(round(tempo_execucao, digits=4)) ms")
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
        println("\n⏳ Tempo médio por imagem: $(round(media_tempo, digits=4)) ms")
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

//otsu original em openmp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>

#define MAX_CINZA 255

const char* DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const char* DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

//----------------------------------------------------------
// Função para calcular o limiar de Otsu com OpenMP
//----------------------------------------------------------
int calcularLimiarOtsuParalelo(const uint8_t* dadosImagem, int largura, int altura) {
    int totalPixeis = largura * altura;
    int histograma[256] = {0};

    // Paraleliza o cálculo do histograma
    #pragma omp parallel
    {
        int hist_local[256] = {0};
        #pragma omp for nowait
        for (int i = 0; i < totalPixeis; i++) {
            hist_local[dadosImagem[i]]++;
        }
        #pragma omp critical
        {
            for (int i = 0; i < 256; i++) {
                histograma[i] += hist_local[i];
            }
        }
    }

    // Cálculo dos vetores cumulativos
    int cumWeight[256] = {0};
    long long cumSum[256] = {0};
    cumWeight[0] = histograma[0];
    cumSum[0] = 0;
    for (int i = 1; i < 256; i++) {
        cumWeight[i] = cumWeight[i - 1] + histograma[i];
        cumSum[i] = cumSum[i - 1] + i * (long long)histograma[i];
    }
    double totalSum = cumSum[255];

    // Busca do limiar ideal
    double maxVariance = 0.0;
    int threshold = 0;
    
    #pragma omp parallel for reduction(max:maxVariance) reduction(+:threshold)
    for (int t = 0; t < 256; t++) {
        int weightB = cumWeight[t];
        int weightF = totalPixeis - weightB;
        
        if (weightB == 0 || weightF == 0) continue;

        double meanB = (double) cumSum[t] / weightB;
        double meanF = (double)(totalSum - cumSum[t]) / weightF;
        double variance = weightB * weightF * (meanB - meanF) * (meanB - meanF);
        
        if (variance > maxVariance) {
            maxVariance = variance;
            threshold = t;
        }
    }
    return threshold;
}

//----------------------------------------------------------
// Função para aplicar a limiarização em OpenMP
//----------------------------------------------------------
void aplicarLimiarizacao(const uint8_t* entrada, uint8_t* saida, int totalPixeis, int limiar) {
    #pragma omp parallel for simd
    for (int i = 0; i < totalPixeis; i++) {
        saida[i] = (entrada[i] > limiar) ? MAX_CINZA : 0;
    }
}

//----------------------------------------------------------
// Função para processar uma única imagem e medir o tempo
//----------------------------------------------------------
double processarImagem(const char* caminhoEntrada, const char* caminhoSaida) {
    double inicio = omp_get_wtime();

    // Carrega a imagem em escala de cinza
    cv::Mat imagem = cv::imread(caminhoEntrada, cv::IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        fprintf(stderr, "Erro ao carregar a imagem: %s\n", caminhoEntrada);
        return 0.0;
    }

    int largura = imagem.cols;
    int altura = imagem.rows;

    // Calcula o limiar de Otsu paralelamente
    int limiarOtsu = calcularLimiarOtsuParalelo(imagem.data, largura, altura);

    // Cria a imagem segmentada e aplica a limiarização
    cv::Mat imagemSegmentada(altura, largura, CV_8UC1);
    aplicarLimiarizacao(imagem.data, imagemSegmentada.data, largura * altura, limiarOtsu);

    // Salva a imagem segmentada
    cv::imwrite(caminhoSaida, imagemSegmentada);

    double tempo_execucao = (omp_get_wtime() - inicio) * 1000.0; // Tempo em milissegundos
    printf("Imagem %s processada em %.4f ms\n", caminhoEntrada, tempo_execucao);
    return tempo_execucao;
}

//----------------------------------------------------------
// Função para processar todas as imagens do diretório (cópia do seu código base)
//----------------------------------------------------------
void processarDiretorio(const char* diretorioEntrada, const char* diretorioSaida) {
    struct dirent *entrada;
    DIR *dp = opendir(diretorioEntrada);
    if (dp == NULL) {
        fprintf(stderr, "Erro ao acessar o diretório: %s\n", diretorioEntrada);
        return;
    }
    struct stat st;
    mkdir(diretorioSaida, 0777); // Cria diretório de saída se não existir

    double tempo_total = 0.0;
    int num_imagens = 0;

    while ((entrada = readdir(dp))) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;
        std::string caminhoEntrada = std::string(diretorioEntrada) + "/" + nome;
        if (stat(caminhoEntrada.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::string caminhoSaida = std::string(diretorioSaida) + "/" + nome;
            double tempo = processarImagem(caminhoEntrada.c_str(), caminhoSaida.c_str());
            tempo_total += tempo;
            num_imagens++;
        }
    }
    closedir(dp);

    if (num_imagens > 0) {
        double media = tempo_total / num_imagens;
        printf("\nTempo médio por imagem: %.4f ms\n", media);
    } else {
        printf("\nNenhuma imagem foi processada.\n");
    }
}

//----------------------------------------------------------
// Função principal
//----------------------------------------------------------
int main() {
    printf("Iniciando processamento com %d threads...\n", omp_get_max_threads());
    processarDiretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA);
    return 0;
}

//otsu original com threads
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>

#define MAX_CINZA 255
#define MAX_THREADS 4  // Ajuste conforme necessário

const char* DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const char* DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

typedef struct {
    const uint8_t* dadosImagem;
    int inicio, fim;
    int histograma[256];
} ThreadData;

//----------------------------------------------------------
// Função para calcular histograma em threads
//----------------------------------------------------------
void* calcularHistogramaThread(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    memset(data->histograma, 0, sizeof(data->histograma));
    for (int i = data->inicio; i < data->fim; i++) {
        data->histograma[data->dadosImagem[i]]++;
    }
    return NULL;
}

//----------------------------------------------------------
// Função para calcular o limiar de Otsu usando `pthread`
//----------------------------------------------------------
int calcularLimiarOtsuParalelo(const uint8_t* dadosImagem, int largura, int altura) {
    int totalPixeis = largura * altura;
    pthread_t threads[MAX_THREADS];
    ThreadData threadData[MAX_THREADS];
    int chunk = totalPixeis / MAX_THREADS;
    
    int histograma[256] = {0};

    // Criar threads para histograma
    for (int t = 0; t < MAX_THREADS; t++) {
        threadData[t].dadosImagem = dadosImagem;
        threadData[t].inicio = t * chunk;
        threadData[t].fim = (t == MAX_THREADS - 1) ? totalPixeis : (t + 1) * chunk;
        pthread_create(&threads[t], NULL, calcularHistogramaThread, &threadData[t]);
    }

    // Esperar threads terminarem
    for (int t = 0; t < MAX_THREADS; t++) {
        pthread_join(threads[t], NULL);
        for (int i = 0; i < 256; i++) {
            histograma[i] += threadData[t].histograma[i];
        }
    }

    // Cálculo dos vetores cumulativos
    int cumWeight[256] = {0};
    long long cumSum[256] = {0};
    cumWeight[0] = histograma[0];
    cumSum[0] = 0;
    for (int i = 1; i < 256; i++) {
        cumWeight[i] = cumWeight[i - 1] + histograma[i];
        cumSum[i] = cumSum[i - 1] + i * (long long)histograma[i];
    }
    double totalSum = cumSum[255];

    // Busca do limiar ideal
    double maxVariance = 0.0;
    int threshold = 0;
    for (int t = 0; t < 256; t++) {
        int weightB = cumWeight[t];
        if (weightB == 0) continue;
        int weightF = totalPixeis - weightB;
        if (weightF == 0) break;

        double meanB = (double)cumSum[t] / weightB;
        double meanF = (double)(totalSum - cumSum[t]) / weightF;
        double variance = weightB * weightF * (meanB - meanF) * (meanB - meanF);
        if (variance > maxVariance) {
            maxVariance = variance;
            threshold = t;
        }
    }
    return threshold;
}

//----------------------------------------------------------
// Função para aplicar a limiarização em threads
//----------------------------------------------------------
void aplicarLimiarizacao(const uint8_t* entrada, uint8_t* saida, int totalPixeis, int limiar) {
    for (int i = 0; i < totalPixeis; i++) {
        saida[i] = (entrada[i] > limiar) ? MAX_CINZA : 0;
    }
}

//----------------------------------------------------------
// Função para processar uma única imagem
//----------------------------------------------------------
double processarImagem(const char* caminhoEntrada, const char* caminhoSaida) {
    double inicio = (double)clock() / CLOCKS_PER_SEC;

    // Carrega a imagem em escala de cinza
    cv::Mat imagem = cv::imread(caminhoEntrada, cv::IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        fprintf(stderr, "Erro ao carregar a imagem: %s\n", caminhoEntrada);
        return 0.0;
    }

    int largura = imagem.cols;
    int altura = imagem.rows;

    // Calcula o limiar de Otsu paralelamente
    int limiarOtsu = calcularLimiarOtsuParalelo(imagem.data, largura, altura);

    // Cria a imagem segmentada e aplica a limiarização
    cv::Mat imagemSegmentada(altura, largura, CV_8UC1);
    aplicarLimiarizacao(imagem.data, imagemSegmentada.data, largura * altura, limiarOtsu);

    // Salva a imagem segmentada
    cv::imwrite(caminhoSaida, imagemSegmentada);

    return ((double)clock() / CLOCKS_PER_SEC - inicio) * 1000.0000; // Retorna tempo em milissegundos
}

//----------------------------------------------------------
// Função para processar todas as imagens do diretório
//----------------------------------------------------------
void processarDiretorio(const char* diretorioEntrada, const char* diretorioSaida) {
    struct dirent *entrada;
    DIR *dp = opendir(diretorioEntrada);
    if (dp == NULL) {
        fprintf(stderr, "Erro ao acessar o diretório: %s\n", diretorioEntrada);
        return;
    }
    struct stat st;
    mkdir(diretorioSaida, 0777); // Cria diretório de saída se não existir

    while ((entrada = readdir(dp))) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;
        std::string caminhoEntrada = std::string(diretorioEntrada) + "/" + nome;
        if (stat(caminhoEntrada.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::string caminhoSaida = std::string(diretorioSaida) + "/" + nome;
            double tempo = processarImagem(caminhoEntrada.c_str(), caminhoSaida.c_str());
            printf("Processada %s em %.4f ms\n", caminhoEntrada.c_str(), tempo);
        }
    }
    closedir(dp);
}

//----------------------------------------------------------
// Função principal
//----------------------------------------------------------
int main() {
    printf("Iniciando processamento com %d threads...\n", MAX_THREADS);
    processarDiretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA);
    return 0;
}
