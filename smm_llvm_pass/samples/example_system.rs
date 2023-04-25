#[global_allocator]
static ALLOC: std::alloc::System = std::alloc::System;

fn swap<'a>(p: &mut &'a Box<i32>, q: &mut &'a Box<i32>) {
    let temp = *p;
    *p = q;
    *q = temp;
}

fn main() {
    let x = Box::new(10);
    let y = Box::new(20);
    let mut xr = &x;
    let mut yr = &y;
    swap(&mut xr, &mut yr);
}