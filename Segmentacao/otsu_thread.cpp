#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <string>
#include <cctype>
#include <thread>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

#define MAX_CINZA 255

const fs::path DIRETORIO_ENTRADA = "/mnt/c/Users/Cliente/OneDrive/Documentos/PIBIC/PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC/Imagens_Selecionadas";
const fs::path DIRETORIO_SAIDA   = "/mnt/c/Users/Cliente/OneDrive/Documentos/PIBIC 2024 - 2025/Saida_Segmentacao";
const int NUM_EXECUCOES = 1;

//----------------------------------------------------------
// Versão paralela do algoritmo de Otsu – otimizada
//----------------------------------------------------------
//
// Nesta versão, apenas a etapa de cálculo do histograma é paralelizada.  
// Foram feitas as seguintes melhorias:
//   1. Utilização de um vetor contíguo (histLocal) para armazenar os histogramas locais de todas as threads,
//      melhorando a localidade de memória e reduzindo overhead de múltiplos vetores.
//   2. A combinação dos histogramas locais é feita de forma simples e direta.
//   3. A etapa de busca do limiar é mantida sequencial, evitando overhead para um loop de apenas 256 iterações.
//


int calcularLimiarOtsuParalelo(const uchar* dadosImagem, int largura, int altura) {
    int totalPixeis = largura * altura;
    unsigned int numThreads = thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 4; 

    int chunkSize = totalPixeis / numThreads;
    
    // Aloca um vetor contíguo para os histogramas locais: cada thread terá 256 inteiros
    vector<int> histLocal(numThreads * 256, 0);
    vector<thread> threads;
    
    // 1. Cálculo do histograma em paralelo:
    // Cada thread processa seu "chunk" de pixels e escreve em seu segmento do vetor histLocal.
    for (unsigned int t = 0; t < numThreads; t++) {
        int start = t * chunkSize;
        int end = (t == numThreads - 1) ? totalPixeis : start + chunkSize;
        threads.push_back(thread([=, &histLocal, dadosImagem]() {
            int offset = t * 256; // posição inicial para a thread t
            for (int i = start; i < end; i++) {
                histLocal[offset + dadosImagem[i]]++;
            }
        }));
    }
    for (auto &thr : threads)
        thr.join();
    
    // Combina os histogramas locais em um histograma global
    int hist[256] = {0};
    for (int i = 0; i < 256; i++) {
        int soma = 0;
        for (unsigned int t = 0; t < numThreads; t++) {
            soma += histLocal[t * 256 + i];
        }
        hist[i] = soma;
    }

    // 2. Cálculo dos vetores cumulativos (peso e soma cumulativa) de forma sequencial.
    int cumWeight[256];
    long long cumSum[256];
    cumWeight[0] = hist[0];
    cumSum[0] = 0; // 0 * hist[0] é 0
    for (int i = 1; i < 256; i++) {
        cumWeight[i] = cumWeight[i - 1] + hist[i];
        cumSum[i] = cumSum[i - 1] + i * (long long) hist[i];
    }
    double totalSum = cumSum[255];

    // 3. Busca do limiar ideal (variância entre classes) de forma sequencial.
    double maxVariance = 0.0;
    int threshold = 0;
    for (int t = 0; t < 256; t++) {
        int weightB = cumWeight[t];
        if (weightB == 0)
            continue;
        int weightF = totalPixeis - weightB;
        if (weightF == 0)
            break;
        double meanB = (double) cumSum[t] / weightB;
        double meanF = (double)(totalSum - cumSum[t]) / weightF;
        double variance = (double) weightB * weightF * (meanB - meanF) * (meanB - meanF);
        if (variance > maxVariance) {
            maxVariance = variance;
            threshold = t;
        }
    }
    return threshold;
}

//----------------------------------------------------------
// Função para aplicar a limiarização à imagem (sequencial)
//----------------------------------------------------------
void aplicarLimiarizacao(const uchar* dadosEntrada, uchar* dadosSaida, int largura, int altura, int limiar) {
    int totalPixeis = largura * altura;
    for (int i = 0; i < totalPixeis; i++) {
        dadosSaida[i] = (dadosEntrada[i] > limiar) ? MAX_CINZA : 0;
    }
}

//----------------------------------------------------------
// Função para verificar se um arquivo possui extensão de imagem suportada
//----------------------------------------------------------
bool verificaImagem(const fs::path& caminho) {
    if (!fs::is_regular_file(caminho))
        return false;
    string ext = caminho.extension().string();
    for (auto &c : ext)
        c = tolower(c);
    return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tiff" || ext == ".tif");
}

//----------------------------------------------------------
// Função para processar uma única imagem
// Essa função carrega a imagem, chama o algoritmo de Otsu paralelizado (apenas histogramas) e salva o resultado.
// O processamento do diretório é feito sequencialmente para facilitar a comparação.
//----------------------------------------------------------
double processarImagem(const fs::path& caminhoEntrada, const fs::path& caminhoSaida) {
    auto inicio = chrono::high_resolution_clock::now();
    
    // Carrega a imagem em escala de cinza
    Mat imagem = imread(caminhoEntrada.string(), IMREAD_GRAYSCALE);
    if (imagem.empty()) {
        cerr << "Erro ao carregar a imagem: " << caminhoEntrada << endl;
        return 0.0;
    }
    
    int largura = imagem.cols;
    int altura = imagem.rows;
    
    // Chama o algoritmo de Otsu paralelizado (com histogramas paralelos)
    int limiarOtsu = calcularLimiarOtsuParalelo(imagem.data, largura, altura);
    
    // Cria a imagem segmentada e aplica a limiarização
    Mat imagemSegmentada(altura, largura, CV_8UC1);
    aplicarLimiarizacao(imagem.data, imagemSegmentada.data, largura, altura, limiarOtsu);
    
    // Salva a imagem segmentada
    if (!imwrite(caminhoSaida.string(), imagemSegmentada)) {
        cerr << "Erro ao salvar a imagem: " << caminhoSaida << endl;
    }
    
    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracao = fim - inicio;
    return duracao.count();
}

//----------------------------------------------------------
// Função para processar todas as imagens de um diretório (processamento sequencial)
//----------------------------------------------------------
vector<double> processarDiretorio(const fs::path& diretorioEntrada, const fs::path& diretorioSaida) {
    vector<double> tempos;
    if (!fs::exists(diretorioSaida))
        fs::create_directories(diretorioSaida);
    for (const auto& entrada : fs::directory_iterator(diretorioEntrada)) {
        if (verificaImagem(entrada.path())) {
            fs::path caminhoEntrada = entrada.path();
            fs::path caminhoSaida = diretorioSaida / caminhoEntrada.filename();
            double tempo = processarImagem(caminhoEntrada, caminhoSaida);
            tempos.push_back(tempo);
            cout << "Processada " << caminhoEntrada.filename().string()
                 << " em " << tempo << " ms" << endl;
        }
    }
    return tempos;
}

//----------------------------------------------------------
// Função para executar múltiplas execuções e exibir as médias
//----------------------------------------------------------
void multiplasExecucoes(const fs::path& diretorioEntrada, const fs::path& diretorioSaida, int execucoes = 1) {
    vector<double> todosTempos;
    for (int i = 0; i < execucoes; i++) {
        cout << "Iniciando execução " << (i + 1) << endl;
        vector<double> tempos = processarDiretorio(diretorioEntrada, diretorioSaida);
        double soma = 0.0;
        for (double t : tempos) {
            soma += t;
            todosTempos.push_back(t);
        }
        if (!tempos.empty()) {
            double media = soma / tempos.size();
            cout << "Média de tempo para a execução " << (i + 1)
                 << ": " << media << " ms" << endl;
        } else {
            cout << "Nenhuma imagem processada nesta execução." << endl;
        }
    }
    if (!todosTempos.empty()) {
        double somaTotal = 0.0;
        for (double t : todosTempos)
            somaTotal += t;
        double mediaGeral = somaTotal / todosTempos.size();
        cout << "Média geral das execuções: " << mediaGeral << " ms" << endl;
    } else {
        cout << "Nenhuma imagem foi processada em nenhuma execução." << endl;
    }
}

int main() {
    multiplasExecucoes(DIRETORIO_ENTRADA, DIRETORIO_SAIDA, NUM_EXECUCOES);
    return 0;
}
