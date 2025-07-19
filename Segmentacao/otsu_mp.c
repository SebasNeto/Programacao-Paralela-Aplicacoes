#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>

#define MAX_NIVEL_CINZA 255

const char* DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const char* DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

//----------------------------------------------------------
// Função para calcular o limiar de Otsu com OpenMP
//----------------------------------------------------------
int calcularLimiar(const uint8_t* dados_imagem, int largura, int altura) {
    int total_pix = largura * altura;
    int histograma[256] = {0};

    // Paraleliza o cálculo do histograma
    #pragma omp parallel
    {
        int histograma_local[256] = {0};
        #pragma omp for nowait
        for (int i = 0; i < total_pix; i++) {
            histograma_local[dados_imagem[i]]++;
        }
        #pragma omp critical
        {
            for (int i = 0; i < 256; i++) {
                histograma[i] += histograma_local[i];
            }
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
    
    #pragma omp parallel for reduction(max:variancia_maxima) reduction(+:limiar)
    for (int t = 0; t < 256; t++) {
        int peso_fundo = peso_cumulativo[t];
        int peso_objeto = total_pix - peso_fundo;
        
        if (peso_fundo == 0 || peso_objeto == 0) continue;

        double media_fundo = (double) soma_cumulativa[t] / peso_fundo;
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
// Função para aplicar a limiarização com OpenMP
//----------------------------------------------------------
void aplicarLimiarizacao(const uint8_t* imagem_entrada, uint8_t* imagem_saida, int total_pix, int limiar) {
    #pragma omp parallel for simd
    for (int i = 0; i < total_pix; i++) {
        imagem_saida[i] = (imagem_entrada[i] > limiar) ? MAX_NIVEL_CINZA : 0;
    }
}

//----------------------------------------------------------
// Função para processar uma única imagem e medir o tempo
//----------------------------------------------------------
double processarImagem(const char* caminho_entrada, const char* caminho_saida) {
    double inicio = omp_get_wtime();

    // Carrega a imagem em escala de cinza
    cv::Mat imagem = cv::imread(caminho_entrada, cv::IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        fprintf(stderr, "Erro ao carregar a imagem: %s\n", caminho_entrada);
        return 0.0;
    }

    int largura = imagem.cols;
    int altura = imagem.rows;

    // Calcula o limiar de Otsu paralelamente
    int limiar_otsu = calcularLimiar(imagem.data, largura, altura);

    // Cria a imagem segmentada e aplica a limiarização
    cv::Mat imagem_segmentada(altura, largura, CV_8UC1);
    aplicarLimiarizacao(imagem.data, imagem_segmentada.data, largura * altura, limiar_otsu);

    // Salva a imagem segmentada
    cv::imwrite(caminho_saida, imagem_segmentada);

    double tempo_execucao = (omp_get_wtime() - inicio) * 1000.0; // Tempo em milissegundos
    printf("Imagem %s processada em %.4f ms\n", caminho_entrada, tempo_execucao);
    return tempo_execucao;
}

//----------------------------------------------------------
// Função para processar todas as imagens do diretório
//----------------------------------------------------------
void processarDiretorio(const char* diretorio_entrada, const char* diretorio_saida) {
    struct dirent *entrada;
    DIR *dp = opendir(diretorio_entrada);
    if (dp == NULL) {
        fprintf(stderr, "Erro ao acessar o diretório: %s\n", diretorio_entrada);
        return;
    }
    struct stat st;
    mkdir(diretorio_saida, 0777); // Cria diretório de saída se não existir

    double tempo_total = 0.0;
    int num_imagens = 0;

    while ((entrada = readdir(dp))) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;
        std::string caminho_entrada = std::string(diretorio_entrada) + "/" + nome;
        if (stat(caminho_entrada.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            std::string caminho_saida = std::string(diretorio_saida) + "/" + nome;
            double tempo = processarImagem(caminho_entrada.c_str(), caminho_saida.c_str());
            tempo_total += tempo;
            num_imagens++;
        }
    }
    closedir(dp);

    if (num_imagens > 0) {
        double tempo_medio = tempo_total / num_imagens;
        printf("\nTempo médio por imagem: %.4f ms\n", tempo_medio);
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
