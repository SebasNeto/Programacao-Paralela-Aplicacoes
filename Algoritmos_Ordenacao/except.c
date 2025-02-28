//quick sort versão 2

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 4  // Número máximo de threads
#define MIN_SIZE 10000 // Tamanho mínimo para criar novas threads

int num_threads = 0;  // Contador de threads ativas
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int *vetor;
    int baixo;
    int alto;
} ThreadArgs;

int particionar(int vetor[], int baixo, int alto) {
    int pivo = vetor[alto];
    int i = baixo - 1;

    for (int j = baixo; j < alto; j++) {
        if (vetor[j] < pivo) {
            i++;
            int temp = vetor[i];
            vetor[i] = vetor[j];
            vetor[j] = temp;
        }
    }

    int temp = vetor[i + 1];
    vetor[i + 1] = vetor[alto];
    vetor[alto] = temp;
    return (i + 1);
}

void *quicksort_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int baixo = args->baixo, alto = args->alto;
    int *vetor = args->vetor;

    if (baixo < alto) {
        int pi = particionar(vetor, baixo, alto);

        // Verifica se o tamanho do subvetor justifica a criação de threads
        int tamanho_esq = pi - baixo;
        int tamanho_dir = alto - pi;

        int criar_threads = 0;
        pthread_mutex_lock(&mutex);
        if (num_threads < MAX_THREADS && (tamanho_esq > MIN_SIZE || tamanho_dir > MIN_SIZE)) {
            num_threads += 2;  // Duas novas threads serão criadas
            criar_threads = 1;
        }
        pthread_mutex_unlock(&mutex);

        if (criar_threads) {
            pthread_t thread_esq, thread_dir;
            ThreadArgs args_esq = {vetor, baixo, pi - 1};
            ThreadArgs args_dir = {vetor, pi + 1, alto};

            pthread_create(&thread_esq, NULL, quicksort_thread, &args_esq);
            pthread_create(&thread_dir, NULL, quicksort_thread, &args_dir);

            pthread_join(thread_esq, NULL);
            pthread_join(thread_dir, NULL);

            pthread_mutex_lock(&mutex);
            num_threads -= 2;  // Libera as threads após a conclusão
            pthread_mutex_unlock(&mutex);
        } else {
            // Executa de forma sequencial se o limite de threads for atingido ou o subvetor for pequeno
            quicksort_thread(&(ThreadArgs){vetor, baixo, pi - 1});
            quicksort_thread(&(ThreadArgs){vetor, pi + 1, alto});
        }
    }

    return NULL;
}

void preencher_vetor(int vetor[], int tamanho) {
    for (int i = 0; i < tamanho; i++)
        vetor[i] = rand() % 100000;
}

int main() {
    int tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;

    srand(time(NULL));

    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho = tamanhos[i];
        int *vetor = (int *)malloc(tamanho * sizeof(int));
        preencher_vetor(vetor, tamanho);

        ThreadArgs args = {vetor, 0, tamanho - 1};

        clock_t inicio = clock();
        quicksort_thread(&args);
        clock_t fim = clock();

        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;

        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        free(vetor);
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}

//quick sorte v2 julia
using Base.Threads, Statistics

# Ajuste desses parâmetros pode ser necessário conforme o seu ambiente.
const MIN_SIZE = 1_000_000                 # Tamanho mínimo para usar paralelismo
const MAX_DEPTH = floor(Int, log2(nthreads()))  # Limita a profundidade de spawn

function particionar!(vetor, baixo, alto)
    pivo = vetor[alto]
    i = baixo - 1
    for j in baixo:(alto - 1)
        if vetor[j] < pivo
            i += 1
            vetor[i], vetor[j] = vetor[j], vetor[i]
        end
    end
    vetor[i + 1], vetor[alto] = vetor[alto], vetor[i + 1]
    return i + 1
end

# Quicksort sequencial para partições pequenas ou quando a profundidade máxima é atingida.
function quicksort_sequencial!(vetor, baixo, alto)
    if baixo < alto
        pi = particionar!(vetor, baixo, alto)
        quicksort_sequencial!(vetor, baixo, pi - 1)
        quicksort_sequencial!(vetor, pi + 1, alto)
    end
end

function quicksort_parallel!(vetor, baixo, alto, depth=0)
    if baixo < alto
        # Se a partição for pequena ou se a profundidade máxima for atingida, usa a versão sequencial
        if (alto - baixo) < MIN_SIZE || depth >= MAX_DEPTH
            quicksort_sequencial!(vetor, baixo, alto)
            return
        end

        pi = particionar!(vetor, baixo, alto)
        # Cria tarefas paralelas para as duas partições
        t1 = @spawn quicksort_parallel!(vetor, baixo, pi - 1, depth + 1)
        t2 = @spawn quicksort_parallel!(vetor, pi + 1, alto, depth + 1)
        fetch(t1)
        fetch(t2)
    end
end

function executar_teste()
    tamanhos = [1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
                75_000_000, 100_000_000, 250_000_000, 500_000_000]
    tempos = Float64[]

    for tamanho in tamanhos
        vetor = rand(1:100_000, tamanho)
        inicio = time()
        quicksort_parallel!(vetor, 1, length(vetor))
        tempo_execucao = round(time() - inicio, digits=4)
        push!(tempos, tempo_execucao)
        println("Tamanho: $tamanho - Tempo: $tempo_execucao segundos")
    end

    media_tempo = round(mean(tempos), digits=4)
    println("\nMédia geral dos tempos de execução: $media_tempo segundos")
end

executar_teste()
