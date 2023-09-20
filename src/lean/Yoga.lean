import Pod.Float
import Pod.Int

open Pod (Float32 Int32)

namespace Yoga

variable {α β : Type}

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

structure Value where
  value : Float32
  unit : Unit

opaque Node.Pointed (α β : Type) : NonemptyType.{0}

structure Node (α β : Type) : Type where
  ref : (Node.Pointed α β).type
  h : Nonempty α

instance [Na : Nonempty α] : Nonempty (Node α β) :=
  .intro { ref := Classical.choice (Node.Pointed α β).property, h := Na }

opaque Config.Pointed (α β : Type) : NonemptyType.{0}

structure Config (α β : Type) : Type where
  ref : (Config.Pointed α β).type
  h : Nonempty β

instance [Na : Nonempty β] : Nonempty (Config α β) :=
  .intro { ref := Classical.choice (Config.Pointed α β).property, h := Na }

def MeasureFunc (α β : Type) : Type :=
  Node α β →
  (width : Float32) → (widthMode : MeasureMode) →
  (height : Float32) → (heightMode : MeasureMode) →
  IO Size

def BaselineFunc (α β : Type) : Type := Node α β → (width height : Float32) → IO Float32
def DirtiedFunc (α β : Type) : Type := Node α β → IO _root_.Unit
def PrintFunc (α β : Type) : Type := Node α β → IO _root_.Unit
def NodeCleanupFunc (α β : Type) : Type := Node α β → IO _root_.Unit
-- def Logger (α β : Type) : Type := Config → Node α β → LogLevel → (format : String) → (args : VaList) → IO Int32
def CloneNodeFunc (α β : Type) : Type := (oldNode owner : Node α β) → (childIndex : Int32) → IO (Node α β)

@[extern "lean_yoga_Node_beq"]
opaque Node.beq (_ _ : @& Node α β) : Bool

instance : BEq (Node α β) := ⟨Node.beq⟩

@[extern "lean_yoga_Node_new"]
opaque Node.new (ctx : α) : BaseIO (Node α β) :=
  pure { ref := Classical.choice (Node.Pointed α β).property, h := .intro ctx }

@[extern "lean_yoga_Node_newWithConfig"]
opaque Node.newWithConfig (ctx : α) (config : @& Config α β) : IO (Node α β)

@[extern "lean_yoga_Node_clone"]
opaque Node.clone (node : @& Node α β) : IO (Node α β)

@[extern "lean_yoga_Node_free"]
opaque Node.free (node : @& Node α β) : IO _root_.Unit

-- todo: don't forget about NOT resetting context in `reset` (is it resetted automatically?)
@[extern "lean_yoga_Node_reset"]
opaque Node.reset (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_insertChild"]
opaque Node.insertChild (node child : @& Node α β) (index : UInt32) : IO _root_.Unit

@[extern "lean_yoga_Node_swapChild"]
opaque Node.swapChild (node child : @& Node α β) (index : UInt32) : IO _root_.Unit

@[extern "lean_yoga_Node_removeChild"]
opaque Node.removeChild (node child : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_removeAllChildren"]
opaque Node.removeAllChildren (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_getChild"]
opaque Node.getChild (node : @& Node α β) (index : UInt32) : IO (Node α β)

@[extern "lean_yoga_Node_getOwner"]
opaque Node.getOwner (node : @& Node α β) : IO (Node α β)

@[extern "lean_yoga_Node_getParent"]
opaque Node.getParent (node : @& Node α β) : IO (Node α β)

@[extern "lean_yoga_Node_getChildCount"]
opaque Node.getChildCount (node : @& Node α β) : IO UInt32

@[extern "lean_yoga_Node_setChildren"]
opaque Node.setChildren (owner : @& Node α β) (children : @& Array (Node α β)) : IO (Node α β)

@[extern "lean_yoga_Node_setIsReferenceBaseline"]
opaque Node.setIsReferenceBaseline (node : @& Node α β) (isReferenceBaseline : Bool) : IO _root_.Unit

@[extern "lean_yoga_Node_isReferenceBaseline"]
opaque Node.isReferenceBaseline (node : @& Node α β) : IO Bool

@[extern "lean_yoga_Node_calculateLayout"]
opaque Node.calculateLayout
  (node : @& Node α β) (availableWidth availableHeight : Float32) (ownerDirection : Direction) :
    IO _root_.Unit

/--
Mark a node as dirty. Only valid for nodes with a custom measure function set.
Yoga knows when to mark all other nodes as dirty but because nodes with measure functions depend
on information not known to Yoga they must perform this dirty marking manually.
-/
@[extern "lean_yoga_Node_markDirty"]
opaque Node.markDirty (node : @& Node α β) : IO _root_.Unit

/--
Marks the current node and all its descendants as dirty.
Intended to be used for Yoga benchmarks. Don't use in production, as calling
`Node.calculateLayout` will cause the recalculation of each and every node.
-/
@[extern "lean_yoga_Node_markDirtyAndPropagateToDescendants"]
opaque Node.markDirtyAndPropagateToDescendants (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_print"]
opaque Node.print (node : @& Node α β) (options : PrintOptions) : IO _root_.Unit

@[extern "lean_yoga_floatIsUndefined"]
opaque floatIsUndefined (value : Float32) : Bool

@[extern "lean_yoga_Node_canUseCachedMeasurement"]
opaque Node.canUseCachedMeasurement
  (widthMode : MeasureMode) (width : Float32)
  (heightMode : MeasureMode) (height : Float32)
  (lastWidthMode : MeasureMode) (lastWidth : Float32)
  (lastHeightMode : MeasureMode) (lastHeight : Float32)
  (lastComputedWidth lastComputedHeight marginRow marginColumn : Float32)
  (config : @& Config) :
    IO Bool

@[extern "lean_yoga_Node_copyStyle"]
opaque Node.copyStyle (dstNode srcNode : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_getContext"]
opaque Node.getContext (node : @& Node α β) : IO α

@[extern "lean_yoga_Node_setContext"]
opaque Node.setContext (node : @& Node α β) (ctx : α) : IO _root_.Unit

@[extern "lean_yoga_Node_getConfig"]
opaque Node.getConfig (node : @& Node α β) : IO (Config α β)

@[extern "lean_yoga_Node_setConfig"]
opaque Node.setConfig (node : @& Node α β) (config : @& Config α β) : IO _root_.Unit

@[extern "lean_yoga_Config_setPrintTreeFlag"]
opaque Config.setPrintTreeFlag (config : @& Config α β) (enabled : Bool) : IO _root_.Unit

@[extern "lean_yoga_Node_hasMeasureFunc"]
opaque Node.hasMeasureFunc (node : @& Node α β) : IO Bool

@[extern "lean_yoga_Node_setMeasureFunc"]
opaque Node.setMeasureFunc (node : @& Node α β) (measureFunc : MeasureFunc α β) : IO _root_.Unit

@[extern "lean_yoga_Node_"]
opaque Node. (node : @& Node α β) : IO 

@[extern "lean_yoga_Node_"]
opaque Node. : IO 

@[extern "lean_yoga_Node_"]
opaque Node. : IO 

@[extern "lean_yoga_Node_"]
opaque Node. : IO 

@[extern "lean_yoga_Node_styleSetDirection"]
opaque Node.styleSetDirection (node : @& Node α β) (direction : Direction) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetDirection"]
opaque Node.styleGetDirection (node : @& Node α β) : IO Direction

@[extern "lean_yoga_Node_styleSetFlexDirection"]
opaque Node.styleSetFlexDirection (node : @& Node α β) (flexDirection : FlexDirection) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlexDirection"]
opaque Node.styleGetFlexDirection (node : @& Node α β) : IO FlexDirection

@[extern "lean_yoga_Node_styleSetJustifyContent"]
opaque Node.styleSetJustifyContent (node : @& Node α β) (justifyContent : Justify) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetJustifyContent"]
opaque Node.styleGetJustifyContent (node : @& Node α β) : IO Justify

@[extern "lean_yoga_Node_styleSetAlignContent"]
opaque Node.styleSetAlignContent (node : @& Node α β) (alignContent : Align) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetAlignContent"]
opaque Node.styleGetAlignContent (node : @& Node α β) : IO Align

@[extern "lean_yoga_Node_styleSetAlignItems"]
opaque Node.styleSetAlignItems (node : @& Node α β) (alignItems : Align) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetAlignItems"]
opaque Node.styleGetAlignItems (node : @& Node α β) : IO Align

@[extern "lean_yoga_Node_styleSetAlignSelf"]
opaque Node.styleSetAlignSelf (node : @& Node α β) (alignSelf : Align) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetAlignSelf"]
opaque Node.styleGetAlignSelf (node : @& Node α β) : IO Align

@[extern "lean_yoga_Node_styleSetPositionType"]
opaque Node.styleSetPositionType (node : @& Node α β) (positionType : PositionType) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetPositionType"]
opaque Node.styleGetPositionType (node : @& Node α β) : IO PositionType

@[extern "lean_yoga_Node_styleSetFlexWrap"]
opaque Node.styleSetFlexWrap (node : @& Node α β) (flexWrap : Wrap) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlexWrap"]
opaque Node.styleGetFlexWrap (node : @& Node α β) : IO Wrap

@[extern "lean_yoga_Node_styleSetOverflow"]
opaque Node.styleSetOverflow (node : @& Node α β) (overflow : Overflow) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetOverflow"]
opaque Node.styleGetOverflow (node : @& Node α β) : IO Overflow

@[extern "lean_yoga_Node_styleSetDisplay"]
opaque Node.styleSetDisplay (node : @& Node α β) (display : Display) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetDisplay"]
opaque Node.styleGetDisplay (node : @& Node α β) : IO Display

@[extern "lean_yoga_Node_styleSetFlex"]
opaque Node.styleSetFlex (node : @& Node α β) (flex : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlex"]
opaque Node.styleGetFlex (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_styleSetFlexGrow"]
opaque Node.styleSetFlexGrow (node : @& Node α β) (flexGrow : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlexGrow"]
opaque Node.styleGetFlexGrow (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_styleSetFlexShrink"]
opaque Node.styleSetFlexShrink (node : @& Node α β) (flexShrink : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlexShrink"]
opaque Node.styleGetFlexShrink (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_styleSetFlexBasis"]
opaque Node.styleSetFlexBasis (node : @& Node α β) (flexBasis : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetFlexBasisPercent"]
opaque Node.styleSetFlexBasisPercent (node : @& Node α β) (flexBasis : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetFlexBasisAuto"]
opaque Node.styleSetFlexBasisAuto (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetFlexBasis"]
opaque Node.styleGetFlexBasis (node : @& Node α β) : IO Value

@[extern "lean_yoga_Node_styleSetPosition"]
opaque Node.styleSetPosition (node : @& Node α β) (edge : Edge) (position : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetPositionPercent"]
opaque Node.styleSetPositionPercent (node : @& Node α β) (edge : Edge) (position : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetPosition"]
opaque Node.styleGetPosition (node : @& Node α β) (edge : Edge) : IO Value

@[extern "lean_yoga_Node_styleSetMargin"]
opaque Node.styleSetMargin (node : @& Node α β) (edge : Edge) (margin : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMarginPercent"]
opaque Node.styleSetMarginPercent (node : @& Node α β) (edge : Edge) (margin : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMarginAuto"]
opaque Node.styleSetMarginAuto (node : @& Node α β) (edge : Edge) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetMargin"]
opaque Node.styleGetMargin (node : @& Node α β) (edge : Edge) : IO Value

@[extern "lean_yoga_Node_styleSetPadding"]
opaque Node.styleSetPadding (node : @& Node α β) (edge : Edge) (padding : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetPaddingPercent"]
opaque Node.styleSetPaddingPercent (node : @& Node α β) (edge : Edge) (padding : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetPadding"]
opaque Node.styleGetPadding (node : @& Node α β) (edge : Edge) : IO Value

@[extern "lean_yoga_Node_styleSetBorder"]
opaque Node.styleSetBorder (node : @& Node α β) (edge : Edge) (border : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetBorder"]
opaque Node.styleGetBorder (node : @& Node α β) (edge : Edge) : IO Float32

@[extern "lean_yoga_Node_styleSetGap"]
opaque Node.styleSetGap (node : @& Node α β) (gutter : Gutter) (gapLength : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetGap"]
opaque Node.styleGetGap (node : @& Node α β) (gutter : Gutter) : IO Float32

@[extern "lean_yoga_Node_styleSetWidth"]
opaque Node.styleSetWidth (node : @& Node α β) (width : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetWidthPercent"]
opaque Node.styleSetWidthPercent (node : @& Node α β) (width : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetWidthAuto"]
opaque Node.styleSetWidthAuto (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetWidth"]
opaque Node.styleGetWidth (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_styleSetHeight"]
opaque Node.styleSetHeight (node : @& Node α β) (height : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetHeightPercent"]
opaque Node.styleSetHeightPercent (node : @& Node α β) (height : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetHeightAuto"]
opaque Node.styleSetHeightAuto (node : @& Node α β) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetHeight"]
opaque Node.styleGetHeight (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_styleSetMinWidth"]
opaque Node.styleSetMinWidth (node : @& Node α β) (minWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMinWidthPercent"]
opaque Node.styleSetMinWidthPercent (node : @& Node α β) (minWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetMinWidth"]
opaque Node.styleGetMinWidth (node : @& Node α β) : IO Value

@[extern "lean_yoga_Node_styleSetMinHeight"]
opaque Node.styleSetMinHeight (node : @& Node α β) (minWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMinHeightPercent"]
opaque Node.styleSetMinHeightPercent (node : @& Node α β) (minWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetMinHeight"]
opaque Node.styleGetMinHeight (node : @& Node α β) : IO Value

@[extern "lean_yoga_Node_styleSetMaxWidth"]
opaque Node.styleSetMaxWidth (node : @& Node α β) (maxWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMaxWidthPercent"]
opaque Node.styleSetMaxWidthPercent (node : @& Node α β) (maxWidth : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetMaxWidth"]
opaque Node.styleGetMaxWidth (node : @& Node α β) : IO Value

@[extern "lean_yoga_Node_styleSetMaxHeight"]
opaque Node.styleSetMaxHeight (node : @& Node α β) (maxHeight : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleSetMaxHeightPercent"]
opaque Node.styleSetMaxHeightPercent (node : @& Node α β) (maxHeight : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetMaxHeight"]
opaque Node.styleGetMaxHeight (node : @& Node α β) : IO Value

/-!
Yoga specific properties, not compatible with flexbox specification Aspect
ratio control the size of the undefined dimension of a node. Aspect ratio is
encoded as a floating point value width/height. e.g. A value of 2 leads to a
node with a width twice the size of its height while a value of 0.5 gives the
opposite effect.

- On a node with a set width/height aspect ratio control the size of the
  unset dimension
- On a node with a set flex basis aspect ratio controls the size of the node
  in the cross axis if unset
- On a node with a measure function aspect ratio works as though the measure
  function measures the flex basis
- On a node with flex grow/shrink aspect ratio controls the size of the node
  in the cross axis if unset
- Aspect ratio takes min/max dimensions into account
-/

@[extern "lean_yoga_Node_styleSetAspectRatio"]
opaque Node.styleSetAspectRatio (node : @& Node α β) (aspectRatio : Float32) : IO _root_.Unit

@[extern "lean_yoga_Node_styleGetAspectRatio"]
opaque Node.styleGetAspectRatio (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetLeft"]
opaque Node.layoutGetLeft (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetTop"]
opaque Node.layoutGetTop (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetRight"]
opaque Node.layoutGetRight (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetBottom"]
opaque Node.layoutGetBottom (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetWidth"]
opaque Node.layoutGetWidth (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetHeight"]
opaque Node.layoutGetHeight (node : @& Node α β) : IO Float32

@[extern "lean_yoga_Node_layoutGetDirection"]
opaque Node.layoutGetDirection (node : @& Node α β) : IO Direction

@[extern "lean_yoga_Node_layoutGetHadOverflow"]
opaque Node.layoutGetHadOverflow (node : @& Node α β) : IO Bool

/-!
Get the computed values for these nodes after performing layout.
If they were set using point values then the returned value will be the same as `Node.styleGetXXX`.
However if they were set using a percentage value then the returned value is
the computed value used during layout.
-/

@[extern "lean_yoga_Node_layoutGetMargin"]
opaque Node.layoutGetMargin (node : @& Node α β) (edge : Edge) : IO Float32

@[extern "lean_yoga_Node_layoutGetBorder"]
opaque Node.layoutGetBorder (node : @& Node α β) (edge : Edge) : IO Float32

@[extern "lean_yoga_Node_layoutGetPadding"]
opaque Node.layoutGetPadding (node : @& Node α β) (edge : Edge) : IO Float32

-- @[extern "lean_yoga_Config_setLogger"]
-- opaque Config.setLogger (config : @& Config) (logger : Logger) : IO _root_.Unit

@[extern "lean_yoga_assert"]
opaque assert (condition : Bool) (message : @& String) : IO _root_.Unit

@[extern "lean_yoga_assertWithNode"]
opaque assertWithNode (node : @& Node α β) (condition : Bool) (message : @& String) : IO _root_.Unit

@[extern "lean_yoga_assertWithConfig"]
opaque assertWithConfig (config : @& Config α β) (condition : Bool) (message : @& String) : IO _root_.Unit

/--
Set this to number of pixels in 1 point to round calculation results.
If you want to avoid rounding - set PointScaleFactor to 0.
-/
@[extern "lean_yoga_Config_setPointScaleFactor"]
opaque Config.setPointScaleFactor (config : @& Config α β) (pixelsInPoint : Float32) : IO _root_.Unit

@[extern "lean_yoga_Config_getPointScaleFactor"]
opaque Config.getPointScaleFactor (config : @& Config α β) : IO Float32

@[extern "lean_yoga_Config_beq"]
opaque Config.beq (_ _ : @& Config α β) : Bool

instance : BEq (Config α β) := ⟨Config.beq⟩

@[extern "lean_yoga_Config_new"]
opaque Config.new (ctx : β) : BaseIO (Config α β) :=
  pure { ref := Classical.choice (Config.Pointed α β).property, h := .intro ctx }

@[extern "lean_yoga_Config_free"]
opaque Config.free (config : @& Config α β) : IO _root_.Unit

@[extern "lean_yoga_Config_copy"]
opaque Config.copy (dest src : @& Config α β) : IO _root_.Unit

@[extern "lean_yoga_Config_getInstanceCount"]
opaque Config.getInstanceCount : BaseIO UInt32

@[extern "lean_yoga_Config_setExperimentalFeatureEnabled"]
opaque Config.setExperimentalFeatureEnabled
  (config : @& Config α β) (feature : ExperimentalFeature) (enabled : Bool) : IO _root_.Unit

@[extern "lean_yoga_Config_isExperimentalFeatureEnabled"]
opaque Config.isExperimentalFeatureEnabled
  (config : @& Config α β) (feature : ExperimentalFeature) : IO Bool

@[extern "lean_yoga_Config_setUseWebDefaults"]
opaque Config.setUseWebDefaults (config : @& Config α β) (enabled : Bool) : IO _root_.Unit

@[extern "lean_yoga_Config_getUseWebDefaults"]
opaque Config.getUseWebDefaults (config : @& Config α β) : IO Bool

@[extern "lean_yoga_Config_setCloneNodeFunc"]
opaque Config.setCloneNodeFunc (config : @& Config α β) (callback : CloneNodeFunc α β) : IO _root_.Unit

@[extern "lean_yoga_Config_getDefault"]
opaque Config.getDefault [Inhabited β] : IO (Config α β)

@[extern "lean_yoga_Config_setErrata"]
opaque Config.setErrata (config : @& Config α β) (errata : Errata) : IO _root_.Unit

@[extern "lean_yoga_Config_getErrata"]
opaque Config.getErrata (config : @& Config α β) : IO Errata

@[extern "lean_yoga_roundValueToPixelGrid"]
opaque roundValueToPixelGrid (value pointScaleFactor : Float) (forceCeil forceFloor : Bool) : Float32
