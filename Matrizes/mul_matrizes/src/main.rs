use rayon::prelude::*;
use std::time::Instant;

fn main() {
    let n = 1000;
    let a: Vec<Vec<f64>> = vec![vec![1.0; n]; n];
    let b: Vec<Vec<f64>> = vec![vec![2.0; n]; n];

    // Início da medição do tempo
    let start = Instant::now();

    // Matriz resultado construída diretamente com iteradores paralelos
    let c: Vec<Vec<f64>> = (0..n)
        .into_par_iter()
        .map(|i| {
            (0..n)
                .map(|j| (0..n).map(|k| a[i][k] * b[k][j]).sum())
                .collect()
        })
        .collect();

    // Tempo total de execução
    let duration = start.elapsed();

    println!("Multiplicação concluída!");
    println!("Tempo de execução: {:.2?} segundos", duration);
}
