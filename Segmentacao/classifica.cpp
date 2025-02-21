#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <cctype>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

string classificarImagens(const Mat& imagem) {
    int largura = imagem.cols;
    int altura = imagem.rows;
    int area = largura * altura;
    
    // Definindo limiares arbitrários:
    // Se a área for menor que 307200 pixels (ex.: 640x480) → "Pequena"
    // Se a área for menor que 921600 pixels (ex.: 1280x720) → "Média"
    // Caso contrário, classifica como "Grande"
    if (area < 307200) {
        return "Pequena";
    } else if (area < 921600) {
        return "Media";
    } else {
        return "Grande";
    }
}

void percorrerDiretorio(const fs::path& caminhoDiretorio) {
    if (!fs::exists(caminhoDiretorio) || !fs::is_directory(caminhoDiretorio)) {
        cout << "Diretório inválido: " << caminhoDiretorio << endl;
        return;
    }
    
    // Itera sobre todos os arquivos do diretório
    for (const auto& entrada : fs::directory_iterator(caminhoDiretorio)) {
        fs::path caminhoArquivo = entrada.path();
        string extensao = caminhoArquivo.extension().string();
        
        // Converte a extensão para minúsculas
        for (auto& c : extensao)
            c = tolower(c);
        
        // Verifica se a extensão corresponde a um dos formatos de imagem suportados
        if (extensao == ".png" || extensao == ".jpg" || extensao == ".jpeg" ||
            extensao == ".bmp" || extensao == ".tiff" || extensao == ".tif") {
            
            // Carrega a imagem
            Mat imagem = imread(caminhoArquivo.string());
            if (imagem.empty()) {
                cout << "Não foi possível carregar a imagem: " << caminhoArquivo << endl;
                continue;
            }
            
            // Classifica a imagem com base na área
            string classificacao = classificarImagens(imagem);
            
            // Exibe o nome da imagem, suas dimensões, área e classificação
            cout << "Imagem: " << caminhoArquivo.filename().string()
                 << " | Dimensões: " << imagem.cols << "x" << imagem.rows
                 << " | Área: " << (imagem.cols * imagem.rows)
                 << " | Classificação: " << classificacao << endl;
        }
    }
}

int main() {
    fs::path diretorioImagens =  "/mnt/c/Users/Cliente/OneDrive/Documentos/PIBIC/PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC/Imagens_Selecionadas";
    percorrerDiretorio(diretorioImagens);
    return 0;
}
