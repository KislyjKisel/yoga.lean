import Pod.Float
import Pod.Int

open Pod (Float32 Int32)

namespace Yoga

inductive Align where
| auto
| flexStart | center | flexEnd
| stretch | baseline | spaceBetween | spaceAround

inductive Dimension where
| width | height

inductive Direction where
| inherit | ltr | rtl

inductive Display where
| flex | none

inductive Edge where
| left | top | right | bottom
| start | end
| horizontal | vertical
| all

structure Errata where
  val : Int32

def Errata.none : Errata := .mk $ .mk 0
def Errata.stretchFlexBasis : Errata := .mk $ .mk 1
def Errata.all : Errata := .mk $ .mk 2147483647
def Errata.classic : Errata := .mk $ .mk 2147483646

instance : Complement Errata where
  complement x := .mk (~~~x.val)

instance : AndOp Errata where
  and x y := .mk (x.val &&& y.val)

instance : OrOp Errata where
  or x y := .mk (x.val ||| y.val)

instance : Xor Errata where
  xor x y := .mk (x.val ^^^ y.val)

inductive ExperimentalFeature where
| webFlexBasis
| absolutePercentageAgainstPaddingEdge
| fixJniLocalRefOverflows

inductive FlexDirection where
| column | columnReverse
| row | rowReverse

inductive Gutter where
| column | row | all

inductive Justify where
| flexStart | center | flexEnd
| spaceBetween | spaceAround | spaceEvenly

inductive LogLevel where
| error | warn | info | debug | verbose | fatal

inductive MeasureMode where
| undefined | exactly | atMost

inductive NodeType where
| default | text

inductive Overflow where
| visible | hidden | scroll

inductive PositionType where
| static | relative | absolute

structure PrintOptions where
  val : Int32

def PrintOptions.layout : PrintOptions := .mk $ .mk 1
def PrintOptions.style : PrintOptions := .mk $ .mk 2
def PrintOptions.children : PrintOptions := .mk $ .mk 4

instance : Complement PrintOptions where
  complement x := .mk (~~~x.val)

instance : AndOp PrintOptions where
  and x y := .mk (x.val &&& y.val)

instance : OrOp PrintOptions where
  or x y := .mk (x.val ||| y.val)

instance : Xor PrintOptions where
  xor x y := .mk (x.val ^^^ y.val)

inductive Unit where
| undefined | point | percent | auto

inductive Wrap where
| noWrap | wrap | wrapReverse

structure Size where
  width : Float32
  height : Float32
deriving Inhabited

opaque Node.Pointed : NonemptyType
def Node := Node.Pointed.type
instance : Nonempty Node := Node.Pointed.property

opaque Config.Pointed : NonemptyType
def Config := Config.Pointed.type
instance : Nonempty Config := Config.Pointed.property

def MeasureFunc : Type :=
  Node →
  (width : Float32) → (widthMode : MeasureMode) →
  (height : Float32) → (heightMode : MeasureMode) →
  IO Size

def BaselineFunc : Type := Node → (width height : Float32) → IO Float32
def DirtiedFunc : Type := Node → IO Unit
def PrintFunc : Type := Node → IO Unit
def NodeCleanupFunc : Type := Node → IO Unit
-- def Logger : Type := Config → Node → LogLevel → (format : String) → (args : VaList) → IO Int32
def CloneNodeFunc : Type := (oldNode owner : Node) → (childIndex : Int32) → IO Node

@[extern "lean_yoga_Node_new"]
opaque Node.new : BaseIO Node

