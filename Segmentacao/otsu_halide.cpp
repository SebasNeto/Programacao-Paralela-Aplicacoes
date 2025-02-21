#include "Halide.h"
#include "halide_image_io.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <vector>

namespace fs = std::filesystem;
using namespace Halide;

// Função para aplicar a limiarização de Otsu em uma imagem
Buffer<uint8_t> aplicar_otsu(const Buffer<uint8_t>& input) {
    // Se a imagem tiver 3 canais (RGB), converte para grayscale
    Buffer<uint8_t> gray;
    if (input.channels() == 3) {
        gray = Buffer<uint8_t>(input.width(), input.height());
        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                // Fórmula para conversão: 0.299*R + 0.587*G + 0.114*B
                float r = input(x, y, 0);
                float g = input(x, y, 1);
                float b = input(x, y, 2);
                gray(x, y) = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
            }
        }
    }
    else {
        // Se já for monocromática, usa diretamente
        gray = input;
    }

    // === Etapa 1: Calcular o histograma usando Halide ===
    Func histogram("histogram");
    Var i;
    // Inicializa o histograma com zero para 256 níveis
    histogram(i) = cast<int32_t>(0);
    RDom r(0, gray.width(), 0, gray.height());
    // Incrementa o histograma no índice correspondente ao valor do pixel
    histogram(gray(r.x, r.y)) += 1;

    // Realiza o cálculo do histograma (buffer de 256 elementos)
    Buffer<int32_t> hist = histogram.realize({ 256 });

    // === Etapa 2: Calcular o limiar de Otsu no host ===
    double total_pixels = gray.width() * gray.height();
    double sum_total = 0.0;
    for (int j = 0; j < 256; j++) {
        sum_total += j * hist(j);
    }

    double sumB = 0.0;
    double wB = 0.0;
    double max_variance = 0.0;
    int threshold = 0;

    for (int t = 0; t < 256; t++) {
        wB += hist(t);
        if (wB == 0)
            continue;

        double wF = total_pixels - wB;
        if (wF == 0)
            break;

        sumB += t * hist(t);
        double mB = sumB / wB;
        double mF = (sum_total - sumB) / wF;
        double variance = wB * wF * (mB - mF) * (mB - mF);

        if (variance > max_variance) {
            max_variance = variance;
            threshold = t;
        }
    }
    std::cout << "Limiar de Otsu calculado: " << threshold << std::endl;

    // === Etapa 3: Binarizar a imagem usando o limiar calculado ===
    Func binary("binary");
    Var x, y;
    binary(x, y) = select(gray(x, y) > threshold, cast<uint8_t>(255), cast<uint8_t>(0));

    // Agendamento para melhor desempenho
    binary.vectorize(x, 8).parallel(y);

    // Realiza a computação da imagem binarizada
    Buffer<uint8_t> output = binary.realize({ gray.width(), gray.height() });

    return output;
}


int main() {
    // Definir diretórios de entrada e saída
    const fs::path DIRETORIO_ENTRADA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC\\PROCESSAMENTO_DE_IMAGENS_PROGAMACAO_PARALELA_PIBIC\\Imagens_Selecionadas";
    const fs::path DIRETORIO_SAIDA = "C:\\Users\\Cliente\\OneDrive\\Documentos\\PIBIC 2024 - 2025\\Saida_Segmentacao";

    // Verificar se os diretórios existem
    if (!fs::exists(DIRETORIO_ENTRADA)) {
        std::cerr << "Diretório de entrada não encontrado!" << std::endl;
        return 1;
    }
    if (!fs::exists(DIRETORIO_SAIDA)) {
        if (!fs::create_directory(DIRETORIO_SAIDA)) {
            std::cerr << "Falha ao criar diretório de saída!" << std::endl;
            return 1;
        }
    }

    // Listar todas as imagens no diretório de entrada
    std::vector<fs::path> imagens;
    for (const auto& entry : fs::directory_iterator(DIRETORIO_ENTRADA)) {
        if (entry.is_regular_file() &&
            (entry.path().extension() == ".png" || entry.path().extension() == ".jpg")) {
            imagens.push_back(entry.path());
        }
    }

    if (imagens.empty()) {
        std::cerr << "Nenhuma imagem encontrada no diretório de entrada!" << std::endl;
        return 1;
    }

    // Processar cada imagem e medir o tempo de execução
    double tempo_total = 0.0;
    for (const auto& caminho_imagem : imagens) {
        // Carregar a imagem
        Buffer<uint8_t> input;
        try {
            input = Tools::load_image(caminho_imagem.string());
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao carregar a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        if (!input.defined()) {
            std::cerr << "Erro: Buffer da imagem " << caminho_imagem.filename()
                << " não foi carregado corretamente." << std::endl;
            continue;
        }

        // --- Execução de aquecimento (primeira execução descartada) ---
        try {
            Buffer<uint8_t> dummy = aplicar_otsu(input);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro na execução de aquecimento para a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        // Medir o tempo da segunda execução
        auto inicio = std::chrono::high_resolution_clock::now();

        Buffer<uint8_t> output;
        try {
            output = aplicar_otsu(input);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao processar a imagem " << caminho_imagem.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        auto fim = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = fim - inicio;
        tempo_total += duracao.count();

        // Salvar a imagem binarizada
        fs::path caminho_saida = DIRETORIO_SAIDA / caminho_imagem.filename();
        try {
            Tools::save_image(output, caminho_saida.string());
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao salvar a imagem " << caminho_saida.filename()
                << ": " << e.what() << std::endl;
            continue;
        }

        std::cout << "Processada: " << caminho_imagem.filename()
            << " | Tempo: " << duracao.count() << "s" << std::endl;
    }

    // Calcular a média do tempo de execução por imagem (considerando somente execuções aquecidas)
    if (!imagens.empty()) {
        double tempo_medio = tempo_total / imagens.size();
        std::cout << "Tempo médio de execução por imagem: "
            << tempo_medio << "s" << std::endl;
    }
    else {
        std::cerr << "Nenhuma imagem foi processada com sucesso." << std::endl;
    }

    return 0;
}

