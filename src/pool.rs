use samp_sdk::types::Cell;
use std::collections::HashMap;

pub struct Pool<T> {
    pub active: HashMap<Cell, Item<T>>,
    current: Cell,
}

pub struct Item<T> {
    pub value: T,
    pub gc: bool,
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
        self.active
            .insert(self.current, Item::<T> { value: t, gc: true });
        self.current
    }
    pub fn alloc_static(&mut self, t: T) -> i32 {
        self.current += 1;
        self.active.insert(
            self.current,
            Item::<T> {
                value: t,
                gc: false,
            },
        );
        self.current
    }
    pub fn get(&mut self, id: i32) -> Option<&mut T> {
        let i: &mut Item<T> = self.active.get_mut(&id)?;
        Some(&mut i.value)
    }
    pub fn get_const(&self, id: i32) -> Option<&T> {
        let i: &Item<T> = self.active.get(&id)?;
        Some(&i.value)
    }
    pub fn set_gc(&mut self, id: i32, set: bool) -> Option<()> {
        self.active.get_mut(&id)?.gc = set;
        Some(())
    }
    pub fn collect(&mut self, id: i32) -> Option<Item<T>> {
        if !self.active.get(&id)?.gc {
            return None;
        }
        self.active.remove(&id)
    }
    pub fn collect_force(&mut self, id: i32) -> Option<Item<T>> {
        self.active.remove(&id)
    }
}
