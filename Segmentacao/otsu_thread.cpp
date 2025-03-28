#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>

#define MAX_NIVEL_CINZA 255
#define NUM_THREADS 4  // Ajuste conforme necessário

const char* DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const char* DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

typedef struct {
    const uint8_t* dados_imagem;
    int inicio, fim;
    int histograma[256];
} DadosThread;

//----------------------------------------------------------
// Função para calcular histograma em threads
//----------------------------------------------------------
void* calcular_histograma_thread(void* arg) {
    DadosThread* dados = (DadosThread*) arg;
    memset(dados->histograma, 0, sizeof(dados->histograma));
    for (int i = dados->inicio; i < dados->fim; i++) {
        dados->histograma[dados->dados_imagem[i]]++;
    }
    return NULL;
}

//----------------------------------------------------------
// Função para calcular o limiar de Otsu usando `pthread`
//----------------------------------------------------------
int calcular_limiar_otsu_paralelo(const uint8_t* dados_imagem, int largura, int altura) {
    int total_pix = largura * altura;
    pthread_t threads[NUM_THREADS];
    DadosThread dados_thread[NUM_THREADS];
    int bloco = total_pix / NUM_THREADS;
    
    int histograma[256] = {0};

    // Criar threads para calcular o histograma
    for (int t = 0; t < NUM_THREADS; t++) {
        dados_thread[t].dados_imagem = dados_imagem;
        dados_thread[t].inicio = t * bloco;
        dados_thread[t].fim = (t == NUM_THREADS - 1) ? total_pix : (t + 1) * bloco;
        pthread_create(&threads[t], NULL, calcular_histograma_thread, &dados_thread[t]);
    }

    // Esperar threads terminarem e combinar os histogramas locais
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
        for (int i = 0; i < 256; i++) {
            histograma[i] += dados_thread[t].histograma[i];
        }
    }

    // Cálculo dos vetores cumulativos
    int peso_cumulativo[256] = {0};
    long long soma_cumulativa[256] = {0};
    peso_cumulativo[0] = histograma[0];
    soma_cumulativa[0] = 0;
    for (int i = 1; i < 256; i++) {
        peso_cumulativo[i] = peso_cumulativo[i - 1] + histograma[i];
        soma_cumulativa[i] = soma_cumulativa[i - 1] + i * (long long)histograma[i];
    }
    double soma_total = soma_cumulativa[255];

    // Busca do limiar ideal
    double variancia_maxima = 0.0;
    int limiar = 0;
    for (int t = 0; t < 256; t++) {
        int peso_fundo = peso_cumulativo[t];
        if (peso_fundo == 0) continue;
        int peso_objeto = total_pix - peso_fundo;
        if (peso_objeto == 0) break;

        double media_fundo = (double)soma_cumulativa[t] / peso_fundo;
        double media_objeto = (double)(soma_total - soma_cumulativa[t]) / peso_objeto;
        double variancia = peso_fundo * peso_objeto * (media_fundo - media_objeto) * (media_fundo - media_objeto);
        if (variancia > variancia_maxima) {
            variancia_maxima = variancia;
            limiar = t;
        }
    }
    return limiar;
}

//----------------------------------------------------------
// Função para aplicar a limiarização
//----------------------------------------------------------
void aplicar_limiarizacao(const uint8_t* imagem_entrada, uint8_t* imagem_saida, int total_pix, int limiar) {
    for (int i = 0; i < total_pix; i++) {
        imagem_saida[i] = (imagem_entrada[i] > limiar) ? MAX_NIVEL_CINZA : 0;
    }
}

//----------------------------------------------------------
// Função para processar uma única imagem
//----------------------------------------------------------
double processar_imagem(const char* caminho_entrada, const char* caminho_saida) {
    double inicio = (double)clock() / CLOCKS_PER_SEC;

    // Carrega a imagem em escala de cinza
    cv::Mat imagem = cv::imread(caminho_entrada, cv::IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        fprintf(stderr, "Erro ao carregar a imagem: %s\n", caminho_entrada);
        return 0.0;
    }

    int largura = imagem.cols;
    int altura = imagem.rows;

    // Calcula o limiar de Otsu paralelamente
    int limiar_otsu = calcular_limiar_otsu_paralelo(imagem.data, largura, altura);

    // Cria a imagem segmentada e aplica a limiarização
    cv::Mat imagem_segmentada(altura, largura, CV_8UC1);
    aplicar_limiarizacao(imagem.data, imagem_segmentada.data, largura * altura, limiar_otsu);

    // Salva a imagem segmentada
    cv::imwrite(caminho_saida, imagem_segmentada);

    return ((double)clock() / CLOCKS_PER_SEC - inicio) * 1000.0; // Retorna tempo em milissegundos
}

//----------------------------------------------------------
// Função para processar todas as imagens do diretório
//----------------------------------------------------------
void processar_diretorio(const char* diretorio_entrada, const char* diretorio_saida) {
    struct dirent *entrada;
    DIR *dp = opendir(diretorio_entrada);
    if (dp == NULL) {
        fprintf(stderr, "Erro ao acessar o diretório: %s\n", diretorio_entrada);
        return;
    }
    struct stat st;
    mkdir(diretorio_saida, 0777); // Cria diretório de saída se não existir

    while ((entrada = readdir(dp))) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;
        std::string caminho_entrada = std::string(diretorio_entrada) + "/" + nome;
        if (stat(caminho_entrada.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::string caminho_saida = std::string(diretorio_saida) + "/" + nome;
            double tempo = processar_imagem(caminho_entrada.c_str(), caminho_saida.c_str());
            printf("Processada %s em %.4f ms\n", caminho_entrada.c_str(), tempo);
        }
    }
    closedir(dp);
}

//----------------------------------------------------------
// Função principal
//----------------------------------------------------------
int main() {
    printf("Iniciando processamento com %d threads...\n", NUM_THREADS);
    processar_diretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA);
    return 0;
}
