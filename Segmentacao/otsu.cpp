#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <string>
#include <cctype>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

#define MAX_CINZA 255

const fs::path DIRETORIO_ENTRADA = "/mnt/c/Users/bicha/Documents/Imagens_Selecionadas";
const fs::path DIRETORIO_SAIDA   = "/mnt/c/Users/bicha/Documents/Saida_otsu";

const int NUM_EXECUCOES = 1; 


// Função que calcula o limiar de Otsu para uma imagem em escala de cinza
int calcularLimiarOtsu(const uchar* dadosImagem, int largura, int altura) {
    int histograma[256] = {0};
    int totalPixeis = largura * altura;

    // Calcula o histograma
    for (int i = 0; i < totalPixeis; i++) {
        histograma[dadosImagem[i]]++;
    }

    double somaTotal = 0.0;
    for (int i = 0; i < 256; i++) {
        somaTotal += i * histograma[i];
    }

    double somaFundo = 0.0;
    int pesoFundo = 0;
    int pesoFrente = 0;
    double varianciaMax = 0.0;
    int limiar = 0;

    // Varre todos os possíveis limiares
    for (int t = 0; t < 256; t++) {
        pesoFundo += histograma[t];
        if (pesoFundo == 0)
            continue;
        pesoFrente = totalPixeis - pesoFundo;
        if (pesoFrente == 0)
            break;
        somaFundo += t * histograma[t];
        double mediaFundo = somaFundo / pesoFundo;
        double mediaFrente = (somaTotal - somaFundo) / pesoFrente;
        double varianciaEntreClasses = (double)pesoFundo * pesoFrente *
                                       (mediaFundo - mediaFrente) * (mediaFundo - mediaFrente);
        if (varianciaEntreClasses > varianciaMax) {
            varianciaMax = varianciaEntreClasses;
            limiar = t;
        }
    }
    return limiar;
}

// Função que aplica a limiarização à imagem, gerando a imagem segmentada
void aplicarLimiriarizacao(const uchar* dadosEntrada, uchar* dadosSaida,
                                     int largura, int altura, int limiar) {
    int totalPixeis = largura * altura;
    for (int i = 0; i < totalPixeis; i++) {
        dadosSaida[i] = (dadosEntrada[i] > limiar) ? MAX_CINZA : 0;
    }
}

// Função que verifica se o arquivo possui uma extensão de imagem suportada
bool verificaImagem(const fs::path& caminho) {
    if (!fs::is_regular_file(caminho))
        return false;
    string ext = caminho.extension().string();
    // Converte para minúsculas
    for (auto &c : ext)
        c = tolower(c);
    return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tiff" || ext == ".tif");
}

// Função que processa uma única imagem e retorna o tempo de execução (em milissegundos)
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

    int limiarOtsu = calcularLimiarOtsu(imagem.data, largura, altura);
    
    // Cria a imagem segmentada
    Mat imagemSegmentada(altura, largura, CV_8UC1);
    aplicarLimiriarizacao(imagem.data, imagemSegmentada.data, largura, altura, limiarOtsu);

    // Salva a imagem segmentada
    if (!imwrite(caminhoSaida.string(), imagemSegmentada)) {
        cerr << "Erro ao salvar a imagem: " << caminhoSaida << endl;
    }

    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracao = fim - inicio;
    return duracao.count();
}

// Acesso ao diretório
vector<double> processarDiretorio(const fs::path& diretorioEntrada,
                                               const fs::path& diretorioSaida) {
    vector<double> temposExecucao;
    if (!fs::exists(diretorioSaida)) {
        fs::create_directories(diretorioSaida);
    }
    for (const auto& entrada : fs::directory_iterator(diretorioEntrada)) {
        if (verificaImagem(entrada.path())) {
            fs::path caminhoEntrada = entrada.path();
            fs::path caminhoSaida = diretorioSaida / caminhoEntrada.filename();
            double tempo = processarImagem(caminhoEntrada, caminhoSaida);
            temposExecucao.push_back(tempo);
            cout << "Tempo de processamento de " 
                 << caminhoEntrada.filename().string() << ": " 
                 << tempo << " ms" << endl;
        }
    }
    return temposExecucao;
}

// Função que executa múltiplas execuções e imprime as médias
void multiplasExecucoes(const fs::path& diretorioEntrada,
                                    const fs::path& diretorioSaida,
                                    int execucoes = 1) {
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
        for (double t : todosTempos) {
            somaTotal += t;
        }
        double mediaGeral = somaTotal / todosTempos.size();
        cout << "Média geral das execuções: " << mediaGeral << " ms" << endl;
    } else {
        cout << "Nenhuma imagem foi processada em nenhuma execução." << endl;
    }
}

int main() {
    // Os diretórios e o número de execuções já estão definidos no código
    multiplasExecucoes(DIRETORIO_ENTRADA, DIRETORIO_SAIDA, NUM_EXECUCOES);
    return 0;
}
