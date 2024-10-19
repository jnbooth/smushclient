pub trait BoolProperty {
    type Target;

    fn get(self, target: &Self::Target) -> bool;
    fn get_mut(self, target: &mut Self::Target) -> &mut bool;
}
