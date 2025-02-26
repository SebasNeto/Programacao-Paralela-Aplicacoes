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
    printf("Imagem %s processada em %.2f ms\n", caminhoEntrada, tempo_execucao);
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
        printf("\nTempo médio por imagem: %.2f ms\n", media);
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
