use recycle_vec::VecExt as _;
use std::ops::{Deref, DerefMut};
use std::{mem, slice};

pub trait HasLifetime {
    type WithLifetime<'a>;
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LifetimeVec<T> {
    inner: Vec<T>,
}

impl<T> Default for LifetimeVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> LifetimeVec<T> {
    pub const fn new() -> Self {
        Self { inner: Vec::new() }
    }
}

impl<T: HasLifetime> LifetimeVec<T> {
    pub fn acquire<'a>(&mut self) -> LifetimeVecInstance<T, T::WithLifetime<'a>> {
        LifetimeVecInstance {
            inner: mem::take(&mut self.inner).recycle(),
            backer: &mut self.inner,
        }
    }
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LifetimeVecInstance<'a, T, U> {
    backer: &'a mut Vec<T>,
    inner: Vec<U>,
}

impl<'a, T, U> Drop for LifetimeVecInstance<'a, T, U> {
    fn drop(&mut self) {
        *self.backer = mem::take(&mut self.inner).recycle();
    }
}

impl<'a, T, U> Deref for LifetimeVecInstance<'a, T, U> {
    type Target = Vec<U>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<'a, T, U> DerefMut for LifetimeVecInstance<'a, T, U> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

impl<'a, 'b, T, U> IntoIterator for &'a LifetimeVecInstance<'b, T, U> {
    type Item = &'a U;

    type IntoIter = slice::Iter<'a, U>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter()
    }
}

impl<'a, 'b, T, U> IntoIterator for &'a mut LifetimeVecInstance<'b, T, U> {
    type Item = &'a mut U;

    type IntoIter = slice::IterMut<'a, U>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter_mut()
    }
}

impl<T> HasLifetime for &'static T {
    type WithLifetime<'a> = &'a T;
}

impl<T> HasLifetime for &'static mut T {
    type WithLifetime<'a> = &'a mut T;
}

macro_rules! impl_lifetime {
    ($t:ty) => {
        impl HasLifetime for $t {
            type WithLifetime<'a> = $t;
        }
    };
}

impl_lifetime!(bool);
impl_lifetime!(u8);
impl_lifetime!(i8);
impl_lifetime!(u16);
impl_lifetime!(i16);
impl_lifetime!(u32);
impl_lifetime!(i32);
impl_lifetime!(u64);
impl_lifetime!(i64);
impl_lifetime!(usize);
impl_lifetime!(isize);
impl_lifetime!(char);

impl<A, B> HasLifetime for (A, B)
where
    A: HasLifetime,
    B: HasLifetime,
{
    type WithLifetime<'a> = (A::WithLifetime<'a>, B::WithLifetime<'a>);
}

impl<A, B, C> HasLifetime for (A, B, C)
where
    A: HasLifetime,
    B: HasLifetime,
    C: HasLifetime,
{
    type WithLifetime<'a> = (
        A::WithLifetime<'a>,
        B::WithLifetime<'a>,
        C::WithLifetime<'a>,
    );
}

impl<A, B, C, D> HasLifetime for (A, B, C, D)
where
    A: HasLifetime,
    B: HasLifetime,
    C: HasLifetime,
    D: HasLifetime,
{
    type WithLifetime<'a> = (
        A::WithLifetime<'a>,
        B::WithLifetime<'a>,
        C::WithLifetime<'a>,
        D::WithLifetime<'a>,
    );
}

impl<A, B, C, D, E> HasLifetime for (A, B, C, D, E)
where
    A: HasLifetime,
    B: HasLifetime,
    C: HasLifetime,
    D: HasLifetime,
    E: HasLifetime,
{
    type WithLifetime<'a> = (
        A::WithLifetime<'a>,
        B::WithLifetime<'a>,
        C::WithLifetime<'a>,
        D::WithLifetime<'a>,
        E::WithLifetime<'a>,
    );
}
