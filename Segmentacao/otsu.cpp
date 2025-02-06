#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#define MAX_CINZA 255

// Função que calcula o limiar 
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

// Função que aplica a limiarização 
void aplicarLimiriarizacao(const uchar* dadosEntrada, uchar* dadosSaida, int largura, int altura, int limiar) {
    int totalPixeis = largura * altura;
    for (int i = 0; i < totalPixeis; i++) {
        dadosSaida[i] = (dadosEntrada[i] > limiar) ? MAX_CINZA : 0;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Uso: " << argv[0] << " <imagem_entrada> <imagem_saida>" << endl;
        return 1;
    }

    Mat imagemEntrada = imread(argv[1], IMREAD_COLOR);
    if (imagemEntrada.empty()) {
        cerr << "Erro ao carregar a imagem: " << argv[1] << endl;
        return 1;
    }

    Mat imagemCinza;
    if (imagemEntrada.channels() == 3) {
        cvtColor(imagemEntrada, imagemCinza, COLOR_BGR2GRAY);
    } else {
        imagemCinza = imagemEntrada;
    }

    int largura = imagemCinza.cols;
    int altura = imagemCinza.rows;

    // Calcula o limiar de Otsu utilizando a função customizada
    int limiarOtsu = calcularLimiarOtsu(imagemCinza.data, largura, altura);
    cout << "Limiar calculado: " << limiarOtsu << endl;

    // Cria a imagem segmentada
    Mat imagemSegmentada(altura, largura, CV_8UC1);
    aplicarLimiriarizacao(imagemCinza.data, imagemSegmentada.data, largura, altura, limiarOtsu);

    // Salva a imagem de saída
    if (!imwrite(argv[2], imagemSegmentada)) {
        cerr << "Erro ao salvar a imagem: " << argv[2] << endl;
        return 1;
    }

    cout << "Imagem segmentada salva em " << argv[2] << endl;
    return 0;
}
