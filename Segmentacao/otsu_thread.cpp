#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>
#include <chrono>

#define MAX_NIVEL_CINZA 255
#define NUM_THREADS 16  

const char* DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const char* DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

// Mutex para sincronizar fusão do histograma
pthread_mutex_t mutex_histograma;

// Estrutura para threads de histograma
typedef struct {
    const uint8_t* dados_imagem;
    int inicio, fim;
    int histograma_local[256];
} DadosThread;

//----------------------------------------------------------
// 🔥 Função para calcular histograma com Mutex
//----------------------------------------------------------
void* calcular_histograma_thread(void* arg) {
    DadosThread* dados = (DadosThread*) arg;
    memset(dados->histograma_local, 0, sizeof(dados->histograma_local));

    for (int i = dados->inicio; i < dados->fim; i++) {
        dados->histograma_local[dados->dados_imagem[i]]++;
    }

    // 🔥 Protege a fusão dos histogramas usando Mutex
    pthread_mutex_lock(&mutex_histograma);
    static int histograma_global[256] = {0};  // Histograma compartilhado entre threads
    for (int i = 0; i < 256; i++) {
        histograma_global[i] += dados->histograma_local[i];
    }
    pthread_mutex_unlock(&mutex_histograma);

    return NULL;
}

//----------------------------------------------------------
// 🔥 Função para calcular o limiar de Otsu com `pthread`
//----------------------------------------------------------
int calcular_limiar_otsu_paralelo(const uint8_t* dados_imagem, int largura, int altura) {
    int total_pix = largura * altura;
    pthread_t threads[NUM_THREADS];
    DadosThread dados_thread[NUM_THREADS];
    int bloco = total_pix / NUM_THREADS;
    
    // 🔥 Inicializa Mutex
    pthread_mutex_init(&mutex_histograma, NULL);

    // Criar threads para calcular histograma
    for (int t = 0; t < NUM_THREADS; t++) {
        dados_thread[t].dados_imagem = dados_imagem;
        dados_thread[t].inicio = t * bloco;
        dados_thread[t].fim = (t == NUM_THREADS - 1) ? total_pix : (t + 1) * bloco;
        pthread_create(&threads[t], NULL, calcular_histograma_thread, &dados_thread[t]);
    }

    // Esperar threads terminarem
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // 🔥 Destrói Mutex após o uso
    pthread_mutex_destroy(&mutex_histograma);

    // 🔥 Cálculo do limiar de Otsu
    int peso_cumulativo[256] = {0};
    long long soma_cumulativa[256] = {0};
    int histograma[256] = {0};

    // Copia o histograma corretamente
    memcpy(histograma, dados_thread[0].histograma_local, sizeof(histograma));

    peso_cumulativo[0] = histograma[0];
    soma_cumulativa[0] = 0;
    for (int i = 1; i < 256; i++) {
        peso_cumulativo[i] = peso_cumulativo[i - 1] + histograma[i];
        soma_cumulativa[i] = soma_cumulativa[i - 1] + i * (long long)histograma[i];
    }

    double soma_total = soma_cumulativa[255];
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
// 🔥 Aplicação do limiar usando `pthread`
//----------------------------------------------------------
void aplicar_limiarizacao(const uint8_t* imagem_entrada, uint8_t* imagem_saida, int total_pix, int limiar) {
    #pragma omp parallel for
    for (int i = 0; i < total_pix; i++) {
        imagem_saida[i] = (imagem_entrada[i] > limiar) ? MAX_NIVEL_CINZA : 0;
    }
}

//----------------------------------------------------------
// 🔥 Processa uma única imagem
//----------------------------------------------------------
double processar_imagem(const char* caminho_entrada, const char* caminho_saida) {
    auto inicio = std::chrono::high_resolution_clock::now();

    cv::Mat imagem = cv::imread(caminho_entrada, cv::IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        fprintf(stderr, "Erro ao carregar a imagem: %s\n", caminho_entrada);
        return 0.0;
    }

    int largura = imagem.cols;
    int altura = imagem.rows;

    int limiar_otsu = calcular_limiar_otsu_paralelo(imagem.data, largura, altura);

    cv::Mat imagem_segmentada(altura, largura, CV_8UC1);
    aplicar_limiarizacao(imagem.data, imagem_segmentada.data, largura * altura, limiar_otsu);

    cv::imwrite(caminho_saida, imagem_segmentada);

    auto fim = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> tempo_execucao = fim - inicio;
    
    printf("Processada %s em %.4f ms\n", caminho_entrada, tempo_execucao.count());
    return tempo_execucao.count();
}

//----------------------------------------------------------
// 🔥 Processa todas as imagens do diretório
//----------------------------------------------------------
void processar_diretorio(const char* diretorio_entrada, const char* diretorio_saida) {
    struct dirent *entrada;
    DIR *dp = opendir(diretorio_entrada);
    if (dp == NULL) {
        fprintf(stderr, "Erro ao acessar o diretório: %s\n", diretorio_entrada);
        return;
    }

    struct stat st;
    mkdir(diretorio_saida, 0777);

    while ((entrada = readdir(dp))) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;
        std::string caminho_entrada = std::string(diretorio_entrada) + "/" + nome;
        if (stat(caminho_entrada.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::string caminho_saida = std::string(diretorio_saida) + "/" + nome;
            processar_imagem(caminho_entrada.c_str(), caminho_saida.c_str());
        }
    }
    closedir(dp);
}

//----------------------------------------------------------
// 🔥 Função principal
//----------------------------------------------------------
int main() {
    printf("Iniciando processamento com %d threads...\n", NUM_THREADS);
    processar_diretorio(DIRETORIO_ENTRADA, DIRETORIO_SAIDA);
    return 0;
}
