use std::io::Read;

const N: usize = 5;

fn main() {
    let mut matrix:Vec<Box<[i32]>> = Vec::with_capacity(N);
    for _ in 0..N {
        matrix.push(vec![0 as i32; N].into_boxed_slice());
    }
    let matrix = matrix.into_boxed_slice();

    let i = get_num();
    let j = get_num();
    if matrix[i][j] > 19222 {
        return;
    }
}

fn get_num() -> usize {
    let mut buf = [0 as u8; 8];
    std::io::stdin().read(&mut buf);
    usize::from_le_bytes(buf)
}