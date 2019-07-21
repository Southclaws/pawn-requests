use std::collections::HashMap;

pub struct Pool<T> {
    pub active: HashMap<i32, T>,
    pub current: i32,
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
        Some(self.active.get_mut(&id)?)
    }
}

pub struct GarbageCollectedPool<T: Clone> {
    pub active: HashMap<i32, Item<T>>,
    current: i32,
}

#[derive(Clone)]
pub struct Item<T: Clone> {
    pub value: T,
    pub gc: bool,
}

impl<T: Clone> Default for GarbageCollectedPool<T> {
    fn default() -> Self {
        GarbageCollectedPool {
            active: HashMap::new(),
            current: 0,
        }
    }
}

impl<T: Clone> GarbageCollectedPool<T> {
    pub fn alloc(&mut self, t: T) -> i32 {
        self.current += 1;
        self.active
            .insert(self.current, Item::<T> { value: t, gc: true });
        self.current
    }

    pub fn get(&mut self, id: i32) -> Option<&mut T> {
        let i: &mut Item<T> = self.active.get_mut(&id)?;
        Some(&mut i.value)
    }

    pub fn take(&mut self, id: i32) -> Option<T> {
        let i = match self.active.get(&id) {
            Some(v) => v.clone(),
            None => return None,
        };
        if i.gc {
            self.active.remove(&id)?;
        }
        Some(i.value)
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
