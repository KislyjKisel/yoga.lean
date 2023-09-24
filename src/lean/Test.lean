import Yoga

open Pod (Float32)
open Yoga

@[extern "lean_yoga_test_Align"]
opaque testAlign
  (auto flexStart center flexEnd : Align)
  (stretch baseline spaceBetween spaceAround : Align) :
    Bool

@[extern "lean_yoga_test_Dimension"]
opaque testDimension (width height : Dimension) : Bool

@[extern "lean_yoga_test_Direction"]
opaque testDirection (inherit ltr rtl : Direction) : Bool

@[extern "lean_yoga_test_Display"]
opaque testDisplay (flex none : Display) : Bool

@[extern "lean_yoga_test_Edge"]
opaque testEdge (left top right bottom start end_ horizontal vertical all : Edge) : Bool

@[extern "lean_yoga_test_Errata"]
opaque testErrata (none stretchFlexBasis all classic : Errata) : Bool

@[extern "lean_yoga_test_ExperimentalFeature"]
opaque testExperimentalFeature
  (webFlexBasis : ExperimentalFeature)
  (absolutePercentageAgainstPaddingEdge : ExperimentalFeature)
  (fixJniLocalRefOverflows : ExperimentalFeature) : Bool

@[extern "lean_yoga_test_FlexDirection"]
opaque testFlexDirection (column columnReverse row rowReverse : FlexDirection) : Bool

@[extern "lean_yoga_test_Gutter"]
opaque testGutter (column row all : Gutter) : Bool

@[extern "lean_yoga_test_Justify"]
opaque testJustify (flexStart center flexEnd spaceBetween spaceAround spaceEvenly : Justify) : Bool

@[extern "lean_yoga_test_LogLevel"]
opaque testLogLevel (error warn info debug verbose fatal : LogLevel) : Bool

@[extern "lean_yoga_test_MeasureMode"]
opaque testMeasureMode (undef exactly atMost : MeasureMode) : Bool

@[extern "lean_yoga_test_NodeType"]
opaque testNodeType (default text : NodeType) : Bool

@[extern "lean_yoga_test_Overflow"]
opaque testOverflow (visible hidden scroll : Overflow) : Bool

@[extern "lean_yoga_test_PositionType"]
opaque testPositionType (static relative absolute : PositionType) : Bool

@[extern "lean_yoga_test_PrintOptions"]
opaque testPrintOptions (layout style children : PrintOptions) : Bool

@[extern "lean_yoga_test_Unit"]
opaque testUnit (undef point percent auto : Yoga.Unit) : Bool

@[extern "lean_yoga_test_Wrap"]
opaque testWrap (noWrap wrap wrapReverse : Wrap) : Bool

def assertRoughlyEqual (name : String) (x y : Float32) : IO Bool :=
  if (x - y).abs < 1e-6
    then pure true
    else
      IO.eprintln s!"{name} failed: {x} ≠ {y}" *>
      pure false

def assertUndefined (name : String) (x : Float32) : IO Bool :=
  if floatIsUndefined x
    then pure true
    else
      IO.eprintln s!"{name} failed: {x} ≠ 0" *>
      pure false

def main : IO UInt32 := do
  let mut allOk := true

  if not $ testAlign .auto .flexStart .center .flexEnd .stretch .baseline .spaceBetween .spaceAround then
    IO.eprintln "`Align` enum has incorrect values"
    allOk := false

  if not $ testDimension .width .height then
    IO.eprintln "`Dimension` enum has incorrect values"
    allOk := false

  if not $ testDirection .inherit .ltr .rtl then
    IO.eprintln "`Direction` enum has incorrect values"
    allOk := false

  if not $ testDisplay .flex .none then
    IO.eprintln "`Display` enum has incorrect values"
    allOk := false

  if not $ testEdge .left .top .right .bottom .start .end .horizontal .vertical .all then
    IO.eprintln "`Edge` enum has incorrect values"
    allOk := false

  if not $ testErrata .none .stretchFlexBasis .all .classic then
    IO.eprintln "`Errata` enum has incorrect values"
    allOk := false

  if not $ testExperimentalFeature
    .webFlexBasis .absolutePercentageAgainstPaddingEdge .fixJniLocalRefOverflows then
      IO.eprintln "`ExperimentalFeature` enum has incorrect values"
      allOk := false

  if not $ testFlexDirection .column .columnReverse .row .rowReverse then
    IO.eprintln "`FlexDirection` enum has incorrect values"
    allOk := false

  if not $ testGutter .column .row .all then
    IO.eprintln "`Gutter` enum has incorrect values"
    allOk := false

  if not $ testJustify .flexStart .center .flexEnd .spaceBetween .spaceAround .spaceEvenly then
    IO.eprintln "`Justify` enum has incorrect values"
    allOk := false

  if not $ testLogLevel .error .warn .info .debug .verbose .fatal then
    IO.eprintln "`LogLevel` enum has incorrect values"
    allOk := false

  if not $ testMeasureMode .undefined .exactly .atMost then
    IO.eprintln "`MeasureMode` enum has incorrect values"
    allOk := false

  if not $ testNodeType .default .text then
    IO.eprintln "`NodeType` enum has incorrect values"
    allOk := false

  if not $ testOverflow .visible .hidden .scroll then
    IO.eprintln "`Overflow` enum has incorrect values"
    allOk := false

  if not $ testPositionType .static .relative .absolute then
    IO.eprintln "`PositionType` enum has incorrect values"
    allOk := false

  if not $ testPrintOptions .layout .style .children then
    IO.eprintln "`PrintOptions` enum has incorrect values"
    allOk := false

  if not $ testUnit .undefined .point .percent .auto then
    IO.eprintln "`Unit` enum has incorrect values"
    allOk := false

  if not $ testWrap .noWrap .wrap .wrapReverse then
    IO.eprintln "`Wrap` enum has incorrect values"
    allOk := false

  /- Adapted from Node Child Test -/
  let root ← Node.new () ()
  let child0 ← Node.new () ()
  child0.styleSetWidth 100
  child0.styleSetHeight 100
  root.insertChild child0 0
  root.calculateLayout undefined undefined .ltr
  allOk := (← assertRoughlyEqual "NodeChildTest:Left1" 0 (← child0.layoutGetLeft)) && allOk
  allOk := (← assertRoughlyEqual "NodeChildTest:Top1" 0 (← child0.layoutGetTop)) && allOk
  allOk := (← assertRoughlyEqual "NodeChildTest:Width1" 100 (← child0.layoutGetWidth)) && allOk
  allOk := (← assertRoughlyEqual "NodeChildTest:Height1" 100 (← child0.layoutGetHeight)) && allOk
  root.removeChild child0
  allOk := (← assertRoughlyEqual "NodeChildTest:Left2" 0 (← child0.layoutGetLeft)) && allOk
  allOk := (← assertRoughlyEqual "NodeChildTest:Top2" 0 (← child0.layoutGetTop)) && allOk
  allOk := (← assertUndefined "NodeChildTest:Width2" (← child0.layoutGetWidth)) && allOk
  allOk := (← assertUndefined "NodeChildTest:Height2" (← child0.layoutGetHeight)) && allOk

  if allOk
    then
      IO.println s!"All OK"
      pure 0
    else
      IO.println s!"Some tests failed"
      pure 1
