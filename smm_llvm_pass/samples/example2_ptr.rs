
#[global_allocator]
static ALLOC: std::alloc::System = std::alloc::System;

fn main() {
    let x = Box::new(29);
    let y = Box::new(39);
    let z: *const i32 = if get_num() % 2 == 0 { &*x } else { &*y };
    unsafe { use_num(z.as_ref().unwrap()); }
}

fn get_num() -> i32 {
    10
}

fn use_num(x: &i32) -> bool {
    if *x > 0 {
        return true;
    }

    return false;
}
