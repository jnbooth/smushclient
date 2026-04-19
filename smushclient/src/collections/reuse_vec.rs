#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub(crate) struct ReuseVec<T> {
    inner: Vec<Option<T>>,
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
        match self.inner.get(pos) {
            Some(Some(item)) => Some(item),
            _ => None,
        }
    }

    pub fn insert(&mut self, timer: T) -> usize {
        if let Some(hole) = self.holes.pop() {
            self.inner[hole] = Some(timer);
            return hole;
        }
        self.inner.push(Some(timer));
        self.inner.len() - 1
    }

    pub fn remove(&mut self, index: usize) -> Option<T> {
        let item = self.inner.get_mut(index)?.take()?;
        self.holes.push(index);
        Some(item)
    }

    pub fn extract_if<'a, P>(&'a mut self, mut pred: P) -> impl Iterator<Item = T> + 'a
    where
        P: FnMut(&T) -> bool + 'a,
    {
        let holes = &mut self.holes;
        self.inner
            .iter_mut()
            .enumerate()
            .filter_map(move |(i, slot)| {
                if !pred(slot.as_ref()?) {
                    return None;
                }
                holes.push(i);
                slot.take()
            })
    }
}
