#[global_allocator]
static ALLOC: std::alloc::System = std::alloc::System;

fn main() {
    let mut a = Vec::new();
    a.push(1);
    a.push(2);
    a.push(3);
    let b = vec![4, 5, 6];
    let c = vec![4, 5, 6];
    let d = vec![4, 5, 6];

    let e = a.get(2).unwrap() + b.get(1).unwrap() + c.get(0).unwrap() + d.get(0).unwrap();
    println!("{}", e);
}