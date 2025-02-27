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
