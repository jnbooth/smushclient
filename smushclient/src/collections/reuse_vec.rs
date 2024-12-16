#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ReuseVec<T> {
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
        ExtractIf {
            holes: &mut self.holes,
            inner: self
                .inner
                .iter_mut()
                .enumerate()
                .filter(move |(_, slot)| match slot {
                    Some(item) => pred(item),
                    None => false,
                }),
        }
    }
}

pub struct ExtractIf<'a, T: 'a, I: Iterator<Item = (usize, &'a mut Option<T>)>> {
    inner: I,
    holes: &'a mut Vec<usize>,
}

impl<'a, T, I> Drop for ExtractIf<'a, T, I>
where
    T: 'a,
    I: Iterator<Item = (usize, &'a mut Option<T>)>,
{
    fn drop(&mut self) {
        for (i, _) in &mut self.inner {
            self.holes.push(i);
        }
    }
}

impl<'a, T, I> Iterator for ExtractIf<'a, T, I>
where
    T: 'a,
    I: Iterator<Item = (usize, &'a mut Option<T>)>,
{
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        let (i, slot) = self.inner.next()?;
        self.holes.push(i);
        slot.take()
    }
}
