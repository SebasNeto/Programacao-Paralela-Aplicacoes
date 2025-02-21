//versão otsu com threads depeseek
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;
using namespace std;
using namespace cv;

// Mutex para sincronizar o acesso ao vetor de resultados
mutex mtx;

// Função para calcular o limiar de Otsu
int calcular_limiar_otsu(const Mat& imagem) {
    // Calcular o histograma
    int histograma[256] = {0};
    for (int y = 0; y < imagem.rows; y++) {
        for (int x = 0; x < imagem.cols; x++) {
            histograma[imagem.at<uchar>(y, x)]++;
        }
    }

    // Calcular o limiar de Otsu
    int total_pixels = imagem.rows * imagem.cols;
    double soma = 0, somaB = 0, wB = 0, wF = 0, max_variancia = 0;
    int limiar = 0;

    for (int i = 0; i < 256; i++) {
        soma += i * histograma[i];
    }

    for (int i = 0; i < 256; i++) {
        wB += histograma[i];
        if (wB == 0) continue;

        wF = total_pixels - wB;
        if (wF == 0) break;

        somaB += i * histograma[i];

        double mediaB = somaB / wB;
        double mediaF = (soma - somaB) / wF;

        double variancia = wB * wF * (mediaB - mediaF) * (mediaB - mediaF);

        if (variancia > max_variancia) {
            max_variancia = variancia;
            limiar = i;
        }
    }

    return limiar;
}

// Função para processar uma imagem
void processar_imagem(const fs::path& caminho_entrada, const fs::path& caminho_saida, double& tempo_processamento) {
    // Carregar a imagem
    Mat imagem = imread(caminho_entrada.string(), IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        cerr << "Erro ao carregar a imagem: " << caminho_entrada.filename() << endl;
        return;
    }

    // Medir o tempo de execução
    auto inicio = chrono::high_resolution_clock::now();

    // Calcular o limiar de Otsu
    int limiar = calcular_limiar_otsu(imagem);

    // Aplicar a binarização
    Mat imagem_binarizada;
    threshold(imagem, imagem_binarizada, limiar, 255, THRESH_BINARY);

    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double> duracao = fim - inicio;
    tempo_processamento = duracao.count();

    // Salvar a imagem binarizada
    if (!imwrite(caminho_saida.string(), imagem_binarizada)) {
        cerr << "Erro ao salvar a imagem: " << caminho_saida.filename() << endl;
    }

    // Exibir o tempo de processamento
    lock_guard<mutex> lock(mtx);
    cout << "Processada: " << caminho_entrada.filename() << " | Limiar: " << limiar << " | Tempo: " << tempo_processamento << "s" << endl;
}

int main() {
    // Definir diretórios de entrada e saída
    const fs::path DIRETORIO_ENTRADA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC\\PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC\\Imagens_Selecionadas";
    const fs::path DIRETORIO_SAIDA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC 2024 - 2025\\Saida_Segmentacao";

    // Verificar se os diretórios existem
    if (!fs::exists(DIRETORIO_ENTRADA)) {
        cerr << "Diretório de entrada não encontrado!" << endl;
        return 1;
    }
    if (!fs::exists(DIRETORIO_SAIDA)) {
        if (!fs::create_directory(DIRETORIO_SAIDA)) {
            cerr << "Falha ao criar diretório de saída!" << endl;
            return 1;
        }
    }

    // Listar todas as imagens no diretório de entrada
    vector<fs::path> imagens;
    for (const auto& entry : fs::directory_iterator(DIRETORIO_ENTRADA)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".png" || entry.path().extension() == ".jpg")) {
            imagens.push_back(entry.path());
        }
    }

    if (imagens.empty()) {
        cerr << "Nenhuma imagem encontrada no diretório de entrada!" << endl;
        return 1;
    }

    // Vetor para armazenar os tempos de processamento
    vector<double> tempos(imagens.size());

    // Vetor de threads
    vector<thread> threads;

    // Processar cada imagem em uma thread separada
    for (size_t i = 0; i < imagens.size(); i++) {
        fs::path caminho_saida = DIRETORIO_SAIDA / imagens[i].filename();
        threads.emplace_back(processar_imagem, imagens[i], caminho_saida, ref(tempos[i]));
    }

    // Aguardar todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    // Calcular a média do tempo de execução
    double tempo_total = 0.0;
    for (double tempo : tempos) {
        tempo_total += tempo;
    }
    double tempo_medio = tempo_total / imagens.size();

    cout << "Tempo médio de execução por imagem: " << tempo_medio << "s" << endl;

    return 0;
}

//versão otsu em c depeseek

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <chrono>

using namespace cv;
namespace fs = std::filesystem;

// Função para calcular o limiar de Otsu
int calcular_limiar_otsu(Mat imagem) {
    int histograma[256] = {0};
    int total_pixels = imagem.rows * imagem.cols;

    // Calcular o histograma
    for (int y = 0; y < imagem.rows; y++) {
        for (int x = 0; x < imagem.cols; x++) {
            histograma[imagem.at<uchar>(y, x)]++;
        }
    }

    // Calcular o limiar de Otsu
    double soma = 0, somaB = 0, wB = 0, wF = 0, max_variancia = 0;
    int limiar = 0;

    for (int i = 0; i < 256; i++) {
        soma += i * histograma[i];
    }

    for (int i = 0; i < 256; i++) {
        wB += histograma[i];
        if (wB == 0)
            continue;

        wF = total_pixels - wB;
        if (wF == 0)
            break;

        somaB += i * histograma[i];

        double mediaB = somaB / wB;
        double mediaF = (soma - somaB) / wF;

        double variancia = wB * wF * (mediaB - mediaF) * (mediaB - mediaF);

        if (variancia > max_variancia) {
            max_variancia = variancia;
            limiar = i;
        }
    }

    return limiar;
}

int main() {
    // Diretórios de entrada e saída
    const fs::path DIRETORIO_ENTRADA = "/mnt/c/Users/Cliente/OneDrive/Documentos/PIBIC/PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC/Imagens_Selecionadas";
    const fs::path DIRETORIO_SAIDA   = "/mnt/c/Users/Cliente/OneDrive/Documentos/PIBIC 2024 - 2025/Saida_Segmentacao";

    // Verificar se o diretório de entrada existe
    if (!fs::exists(DIRETORIO_ENTRADA)) {
        std::cerr << "Diretório de entrada não encontrado: " << DIRETORIO_ENTRADA << std::endl;
        return -1;
    }
    
    // Criar o diretório de saída, se necessário
    if (!fs::exists(DIRETORIO_SAIDA)) {
        if (!fs::create_directories(DIRETORIO_SAIDA)) {
            std::cerr << "Erro ao criar o diretório de saída: " << DIRETORIO_SAIDA << std::endl;
            return -1;
        }
    }

    // Variáveis para medir o tempo total e contar as imagens processadas
    double tempo_total = 0.0;
    int contagem_imagens = 0;

    // Iterar sobre os arquivos do diretório de entrada
    for (const auto& entry : fs::directory_iterator(DIRETORIO_ENTRADA)) {
        if (entry.is_regular_file()) {
            fs::path caminhoImagem = entry.path();
            // Verificar se a extensão é compatível (.jpg, .jpeg ou .png)
            if (caminhoImagem.extension() == ".jpg" ||
                caminhoImagem.extension() == ".jpeg" ||
                caminhoImagem.extension() == ".png") {

                std::cout << "Processando: " << caminhoImagem << std::endl;

                // Inicia a medição do tempo para esta imagem
                auto inicio = std::chrono::high_resolution_clock::now();
                
                // Carregar a imagem em tons de cinza
                Mat imagem = imread(caminhoImagem.string(), IMREAD_GRAYSCALE);
                if (imagem.empty()) {
                    std::cerr << "Erro ao carregar a imagem: " << caminhoImagem << std::endl;
                    continue;
                }

                // Calcular o limiar de Otsu
                int limiar = calcular_limiar_otsu(imagem);
                std::cout << "Limiar de Otsu calculado para " << caminhoImagem.filename() << ": " << limiar << std::endl;

                // Aplicar a binarização
                Mat imagem_binarizada;
                threshold(imagem, imagem_binarizada, limiar, 255, THRESH_BINARY);

                // Construir o caminho completo para salvar a imagem binarizada
                fs::path caminhoSaida = DIRETORIO_SAIDA / caminhoImagem.filename();
                if (!imwrite(caminhoSaida.string(), imagem_binarizada)) {
                    std::cerr << "Erro ao salvar a imagem: " << caminhoSaida << std::endl;
                } else {
                    std::cout << "Imagem salva com sucesso: " << caminhoSaida << std::endl;
                }
                
                // Finaliza a medição do tempo para esta imagem
                auto fim = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duracao = fim - inicio;
                tempo_total += duracao.count();
                contagem_imagens++;

                std::cout << "Tempo de processamento para " << caminhoImagem.filename() 
                          << ": " << duracao.count() << " segundos.\n" << std::endl;
            }
        }
    }

    // Exibir a média de tempo de processamento por imagem
    if (contagem_imagens > 0) {
        double tempo_medio = tempo_total / contagem_imagens;
        std::cout << "Tempo médio de processamento por imagem: " 
                  << tempo_medio << " segundos." << std::endl;
    } else {
        std::cerr << "Nenhuma imagem foi processada." << std::endl;
    }

    return 0;
}
