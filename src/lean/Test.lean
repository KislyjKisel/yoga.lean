import Yoga

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

def main : IO UInt32 := do
  if not $ testAlign .auto .flexStart .center .flexEnd .stretch .baseline .spaceBetween .spaceAround then
    IO.eprintln "`Align` enum has incorrect values"
    return 1

  if not $ testDimension .width .height then
    IO.eprintln "`Dimension` enum has incorrect values"
    return 1

  if not $ testDirection .inherit .ltr .rtl then
    IO.eprintln "`Direction` enum has incorrect values"
    return 1

  if not $ testDisplay .flex .none then
    IO.eprintln "`Display` enum has incorrect values"
    return 1

  if not $ testEdge .left .top .right .bottom .start .end .horizontal .vertical .all then
    IO.eprintln "`Edge` enum has incorrect values"
    return 1

  if not $ testErrata .none .stretchFlexBasis .all .classic then
    IO.eprintln "`Errata` enum has incorrect values"
    return 1

  if not $ testExperimentalFeature
    .webFlexBasis .absolutePercentageAgainstPaddingEdge .fixJniLocalRefOverflows then
      IO.eprintln "`ExperimentalFeature` enum has incorrect values"
      return 1

  if not $ testFlexDirection .column .columnReverse .row .rowReverse then
    IO.eprintln "`FlexDirection` enum has incorrect values"
    return 1

  if not $ testGutter .column .row .all then
    IO.eprintln "`Gutter` enum has incorrect values"
    return 1

  if not $ testJustify .flexStart .center .flexEnd .spaceBetween .spaceAround .spaceEvenly then
    IO.eprintln "`Justify` enum has incorrect values"
    return 1

  if not $ testLogLevel .error .warn .info .debug .verbose .fatal then
    IO.eprintln "`LogLevel` enum has incorrect values"
    return 1

  if not $ testMeasureMode .undefined .exactly .atMost then
    IO.eprintln "`MeasureMode` enum has incorrect values"
    return 1

  if not $ testNodeType .default .text then
    IO.eprintln "`NodeType` enum has incorrect values"
    return 1

  if not $ testOverflow .visible .hidden .scroll then
    IO.eprintln "`Overflow` enum has incorrect values"
    return 1

  if not $ testPositionType .static .relative .absolute then
    IO.eprintln "`PositionType` enum has incorrect values"
    return 1

  if not $ testPrintOptions .layout .style .children then
    IO.eprintln "`PrintOptions` enum has incorrect values"
    return 1

  if not $ testUnit .undefined .point .percent .auto then
    IO.eprintln "`Unit` enum has incorrect values"
    return 1

  if not $ testWrap .noWrap .wrap .wrapReverse then
    IO.eprintln "`Wrap` enum has incorrect values"
    return 1

  IO.println s!"All OK"
  pure 0
