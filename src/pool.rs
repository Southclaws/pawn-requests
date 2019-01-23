use samp_sdk::types::Cell;
use std::collections::HashMap;

pub struct Pool<T> {
    pub active: HashMap<Cell, T>,
    current: Cell,
}

impl<T> Default for Pool<T> {
    fn default() -> Self {
        Pool {
            active: HashMap::new(),
            current: 0,
        }
    }
}

impl<T> Pool<T> {
    pub fn alloc(&mut self, t: T) -> i32 {
        self.current += 1;
        self.active.insert(self.current, t);
        self.current
    }
    pub fn get(&mut self, id: i32) -> Option<&mut T> {
        self.active.get_mut(&id)
    }
}
