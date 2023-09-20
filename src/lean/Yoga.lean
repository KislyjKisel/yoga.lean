import Pod.Float

namespace Yoga

structure Size where
  width : Float32
  height : Float32

opaque Node.Pointed : NonemptyType
def Node := Node.Pointed.type
instance : Nonempty Node := Node.Pointed.property

@[extern "lean_yoga_Node_new"]
opaque Node.new : BaseIO Node

