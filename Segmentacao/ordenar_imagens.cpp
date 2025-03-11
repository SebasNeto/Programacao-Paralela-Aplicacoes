#include "Halide.h"
#include "halide_image_io.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <iostream>
#include <omp.h>
#include <fstream>

namespace fs = std::filesystem;
using namespace Halide;
using namespace Halide::Tools;

// Diretórios de entrada e saída
const std::string DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const std::string DIRETORIO_SAIDA = "/mnt/c/Users/bicha/Documents/Saida_otsu";

// Estrutura para armazenar atributos e caminho da imagem
struct ImagemInfo {
    float atributo;
    std::string caminho;
};

//----------------------------------------------------------
// Função para calcular histograma usando Halide
//----------------------------------------------------------
std::vector<int> calcular_histograma(const Halide::Buffer<uint8_t> &imagem) {
    Func histograma("histograma");
    Var i;
    
    // Inicializa o histograma
    histograma(i) = cast<int32_t>(0);
    
    // Domínio de redução sobre todos os pixels da imagem
    RDom r(0, imagem.width(), 0, imagem.height());
    histograma(imagem(r.x, r.y)) += 1;
    
    // Realiza o cálculo do histograma
    Halide::Buffer<int32_t> hist = histograma.realize({256});
    
    // Converter para vetor padrão de C++
    std::vector<int> hist_vetor(256);
    for (int j = 0; j < 256; j++) {
        hist_vetor[j] = hist(j);
    }
    return hist_vetor;
}

//----------------------------------------------------------
// Função para calcular a entropia de uma imagem
//----------------------------------------------------------
float calcular_entropia(const std::vector<int> &histograma, int total_pixels) {
    float entropia = 0.0;
    for (int i = 0; i < 256; i++) {
        if (histograma[i] > 0) {
            float prob = (float)histograma[i] / total_pixels;
            entropia -= prob * log2(prob);
        }
    }
    return entropia;
}

//----------------------------------------------------------
// Função para extrair atributo da imagem (histograma + entropia)
//----------------------------------------------------------
float extrair_atributo(const std::string &caminho_imagem) {
    // Carregar imagem usando OpenCV e converter para escala de cinza
    cv::Mat img_cv = cv::imread(caminho_imagem, cv::IMREAD_GRAYSCALE);
    if (img_cv.empty()) {
        std::cerr << "Erro ao carregar a imagem: " << caminho_imagem << std::endl;
        return -1;
    }

    // Converter OpenCV para Halide
    Halide::Buffer<uint8_t> imagem_halide(img_cv.cols, img_cv.rows);
    for (int y = 0; y < img_cv.rows; y++)
        for (int x = 0; x < img_cv.cols; x++)
            imagem_halide(x, y) = img_cv.at<uint8_t>(y, x);

    // Calcular histograma
    std::vector<int> histograma = calcular_histograma(imagem_halide);

    // Calcular entropia
    float entropia = calcular_entropia(histograma, img_cv.rows * img_cv.cols);

    return entropia;
}

//----------------------------------------------------------
// Funções para Ordenação Paralela (Merge Sort com OpenMP)
//----------------------------------------------------------
void merge(std::vector<ImagemInfo> &imagens, int inicio, int meio, int fim) {
    std::vector<ImagemInfo> temp(fim - inicio + 1);
    int i = inicio, j = meio + 1, k = 0;

    while (i <= meio && j <= fim) {
        if (imagens[i].atributo <= imagens[j].atributo) temp[k++] = imagens[i++];
        else temp[k++] = imagens[j++];
    }
    while (i <= meio) temp[k++] = imagens[i++];
    while (j <= fim) temp[k++] = imagens[j++];
    for (i = inicio, k = 0; i <= fim; i++, k++) imagens[i] = temp[k];
}

void merge_sort(std::vector<ImagemInfo> &imagens, int inicio, int fim) {
    if (inicio < fim) {
        int meio = inicio + (fim - inicio) / 2;
        #pragma omp parallel sections
        {
            #pragma omp section
            merge_sort(imagens, inicio, meio);
            #pragma omp section
            merge_sort(imagens, meio + 1, fim);
        }
        merge(imagens, inicio, meio, fim);
    }
}

//----------------------------------------------------------
// Função para processar todas as imagens
//----------------------------------------------------------
void processar_imagens() {
    std::vector<ImagemInfo> lista_imagens;
    
    for (const auto &entrada : fs::directory_iterator(DIRETORIO_ENTRADA)) {
        if (entrada.is_regular_file() && 
            (entrada.path().extension() == ".png" || entrada.path().extension() == ".jpg")) {
            
            float atributo = extrair_atributo(entrada.path().string());
            if (atributo >= 0) {
                lista_imagens.push_back({atributo, entrada.path().string()});
            }
        }
    }

    std::cout << "Antes da ordenação:\n";
    for (const auto& img : lista_imagens) {
        std::cout << "Imagem: " << img.caminho << " - Entropia: " << img.atributo << "\n";
    }


    // Ordenar imagens paralelamente
    merge_sort(lista_imagens, 0, lista_imagens.size() - 1);

    std::cout << "\nDepois da ordenação:\n";
    for (const auto& img : lista_imagens) {
        std::cout << "Imagem: " << img.caminho << " - Entropia: " << img.atributo << "\n";
    }

    // // Salvar entropias para análise antes de copiar as imagens ordenadas
    // std::ofstream arquivo("entropias.csv");
    // arquivo << "Nome,Entropia\n";
    // for (const auto& img : lista_imagens) {
    //     arquivo << img.caminho << "," << img.atributo << "\n";
    // }
    // arquivo.close();
    // std::cout << "Entropias salvas em entropias.csv\n";

    // Criar diretório de saída
    fs::create_directory(DIRETORIO_SAIDA);

    // Salvar imagens ordenadas
    for (size_t i = 0; i < lista_imagens.size(); i++) {
        fs::path novo_caminho = fs::path(DIRETORIO_SAIDA) / ("img_" + std::to_string(i) + ".png");
        fs::copy(lista_imagens[i].caminho, novo_caminho);
    }

    std::cout << "Imagens ordenadas e salvas em: " << DIRETORIO_SAIDA << std::endl;
}

//----------------------------------------------------------
// Função principal
//----------------------------------------------------------
int main() {
    std::cout << "Iniciando processamento de imagens com " << omp_get_max_threads() << " threads...\n";
    processar_imagens();
    return 0;
}
