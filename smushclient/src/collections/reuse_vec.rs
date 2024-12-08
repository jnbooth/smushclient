#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ReuseVec<T> {
    inner: Vec<T>,
    holes: Vec<usize>,
}

impl<T> Default for ReuseVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> ReuseVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: Vec::new(),
            holes: Vec::new(),
        }
    }

    pub fn clear(&mut self) {
        self.inner.clear();
        self.holes.clear();
    }

    pub fn get(&self, pos: usize) -> Option<&T> {
        self.inner.get(pos)
    }

    pub fn insert(&mut self, timer: T) -> usize {
        let Some(hole) = self.holes.pop() else {
            self.inner.push(timer);
            return self.inner.len() - 1;
        };
        self.inner[hole] = timer;
        hole
    }

    pub fn remove(&mut self, index: usize) {
        if index >= self.holes.len() {
            return;
        }
        self.holes.push(index);
    }
}
