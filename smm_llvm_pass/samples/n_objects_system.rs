#[global_allocator]
static ALLOC: std::alloc::System = std::alloc::System;

fn swap<'a>(p: &mut &'a Box<i32>, q: &mut &'a Box<i32>) {
    let temp = *p;
    *p = q;
    *q = temp;
}

fn main() {
    let x0 = Box::new(10);
    let x1 = Box::new(10);
    let x2 = Box::new(10);
    let x3 = Box::new(10);
    
    let mut xr0 = &x0;
    let mut xr1 = &x1;
    let mut xr2 = &x2;
    let mut xr3 = &x3;

    swap(&mut xr0, &mut xr1);
    swap(&mut xr2, &mut xr3);
}