#include <lean/lean.h>
#include <yoga/Yoga.h>

/// @param sz must be divisible by `LEAN_OBJECT_SIZE_DELTA`
static inline void* lean_yoga_alloc(size_t sz) {
#ifdef LEAN_YOGA_ALLOC_NATIVE
    return malloc(sz);
#else
    return (void*)lean_alloc_small_object(sz);
#endif
}

/// @param p pointer to memory allocated with `lean_raylib_alloc`
static inline void lean_yoga_free(void* p) {
#ifdef LEAN_YOGA_ALLOC_NATIVE
    free(p);
#else
    lean_free_small_object((lean_object*)p);
#endif
}


// # Tests

#ifndef LEAN_YOGA_SKIP_TESTS

LEAN_EXPORT uint8_t lean_yoga_test_Align(
    uint8_t auto_, uint8_t flexStart, uint8_t center, uint8_t flexEnd, 
    uint8_t stretch, uint8_t baseline, uint8_t spaceBetween, uint8_t spaceAround
) {
    return
        auto_ == YGAlignAuto &&
        flexStart == YGAlignFlexStart &&
        center == YGAlignCenter &&
        flexEnd == YGAlignFlexEnd &&
        stretch == YGAlignStretch &&
        baseline == YGAlignBaseline &&
        spaceBetween == YGAlignSpaceBetween &&
        spaceAround == YGAlignSpaceAround;
}

LEAN_EXPORT uint8_t lean_yoga_test_Dimension(uint8_t width, uint8_t height) {
    return YGDimensionWidth == width && YGDimensionHeight == height;
}

LEAN_EXPORT uint8_t lean_yoga_test_Direction(uint8_t inherit, uint8_t ltr, uint8_t rtl) {
    return
        YGDirectionInherit == inherit &&
        YGDirectionLTR == ltr &&
        YGDirectionRTL == rtl;
}

LEAN_EXPORT uint8_t lean_yoga_test_Display(uint8_t flex, uint8_t none) {
    return YGDisplayFlex == flex && YGDisplayNone == none;
}

LEAN_EXPORT uint8_t lean_yoga_test_Edge(
    uint8_t left, uint8_t top, uint8_t right, uint8_t bottom,
    uint8_t start, uint8_t end, uint8_t horizontal, uint8_t vertical,
    uint8_t all
) {
    return
        YGEdgeLeft == left &&
        YGEdgeTop == top &&
        YGEdgeRight == right &&
        YGEdgeBottom == bottom &&
        YGEdgeStart == start &&
        YGEdgeEnd == end &&
        YGEdgeHorizontal == horizontal &&
        YGEdgeVertical == vertical &&
        YGEdgeAll == all;
}

LEAN_EXPORT uint8_t lean_yoga_test_Errata(
    uint32_t none, uint32_t stretchFlexBasis, uint32_t all, uint32_t classic
) {
    return
        YGErrataNone == (int32_t)none &&
        YGErrataStretchFlexBasis == (int32_t)stretchFlexBasis &&
        YGErrataAll == (int32_t)all &&
        YGErrataClassic == (int32_t)classic;
}

LEAN_EXPORT uint8_t lean_yoga_test_ExperimentalFeature(
    uint8_t webFlexBasis, uint8_t absolutePercentageAgainstPaddingEdge, uint8_t fixJniLocalRefOverflows
) {
    return
        YGExperimentalFeatureWebFlexBasis == webFlexBasis &&
        YGExperimentalFeatureAbsolutePercentageAgainstPaddingEdge == absolutePercentageAgainstPaddingEdge &&
        YGExperimentalFeatureFixJNILocalRefOverflows == fixJniLocalRefOverflows;
}

LEAN_EXPORT uint8_t lean_yoga_test_FlexDirection(
    uint8_t column, uint8_t columnReverse, uint8_t row, uint8_t rowReverse
) {
    return
        YGFlexDirectionColumn == column &&
        YGFlexDirectionColumnReverse == columnReverse &&
        YGFlexDirectionRow == row &&
        YGFlexDirectionRowReverse == rowReverse;
}

LEAN_EXPORT uint8_t lean_yoga_test_Gutter(uint8_t column, uint8_t row, uint8_t all) {
    return
        YGGutterColumn == column &&
        YGGutterRow == row &&
        YGGutterAll == all;
}

LEAN_EXPORT uint8_t lean_yoga_test_Justify(
    uint8_t flexStart, uint8_t center, uint8_t flexEnd,
    uint8_t spaceBetween, uint8_t spaceAround, uint8_t spaceEvenly
) {
    return
        YGJustifyFlexStart == flexStart &&
        YGJustifyCenter == center &&
        YGJustifyFlexEnd == flexEnd &&
        YGJustifySpaceBetween == spaceBetween &&
        YGJustifySpaceAround == spaceAround &&
        YGJustifySpaceEvenly == spaceEvenly;
}

LEAN_EXPORT uint8_t lean_yoga_test_LogLevel(
    uint8_t error, uint8_t warn, uint8_t info, uint8_t debug, uint8_t verbose, uint8_t fatal
) {
    return
        YGLogLevelError == error &&
        YGLogLevelWarn == warn &&
        YGLogLevelInfo == info &&
        YGLogLevelDebug == debug &&
        YGLogLevelVerbose == verbose &&
        YGLogLevelFatal == fatal;
}

LEAN_EXPORT uint8_t lean_yoga_test_MeasureMode(uint8_t undef, uint8_t exactly, uint8_t atMost) {
    return
        YGMeasureModeUndefined == undef &&
        YGMeasureModeExactly == exactly &&
        YGMeasureModeAtMost == atMost;
}

LEAN_EXPORT uint8_t lean_yoga_test_NodeType(uint8_t default_, uint8_t text) {
    return YGNodeTypeDefault == default_ && YGNodeTypeText == text;
}

LEAN_EXPORT uint8_t lean_yoga_test_Overflow(uint8_t visible, uint8_t hidden, uint8_t scroll) {
    return
        YGOverflowVisible == visible &&
        YGOverflowHidden == hidden &&
        YGOverflowScroll == scroll;
}

LEAN_EXPORT uint8_t lean_yoga_test_PositionType(uint8_t static_, uint8_t relative, uint8_t absolute) {
    return
        YGPositionTypeStatic == static_ &&
        YGPositionTypeRelative == relative &&
        YGPositionTypeAbsolute == absolute;
}

LEAN_EXPORT uint8_t lean_yoga_test_PrintOptions(uint32_t layout, uint32_t style, uint32_t children) {
    return
        YGPrintOptionsLayout == (int32_t)layout &&
        YGPrintOptionsStyle == (int32_t)style &&
        YGPrintOptionsChildren == (int32_t)children;
}

LEAN_EXPORT uint8_t lean_yoga_test_Unit(uint8_t undef, uint8_t point, uint8_t percent, uint8_t auto_) {
    return
        YGUnitUndefined == undef &&
        YGUnitPoint == point &&
        YGUnitPercent == percent &&
        YGUnitAuto == auto_;
}

LEAN_EXPORT uint8_t lean_yoga_test_Wrap(uint8_t noWrap, uint8_t wrap, uint8_t wrapReverse) {
    return
        YGWrapNoWrap == noWrap &&
        YGWrapWrap == wrap &&
        YGWrapWrapReverse == wrapReverse;
}

#endif
