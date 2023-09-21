#include <lean/lean.h>
#include <lean_pod.h>
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

// TODO: children as a flexible array
typedef struct {
    lean_object* value;
    lean_object* parent;
    lean_object* config;
    lean_object** children;
    size_t childrenCapacity;
} lean_yoga_Node_context;

typedef struct {
    lean_object* value;
} lean_yoga_Config_context;

static lean_external_class* lean_yoga_Node_class = NULL;
static lean_external_class* lean_yoga_Config_class = NULL;

static void lean_yoga_Node_foreach(void* node, b_lean_obj_arg f) {
    lean_yoga_Node_context* ctx = YGNodeGetContext((YGNodeRef)node);
    size_t childCount = YGNodeGetChildCount((YGNodeRef)node);
    lean_inc_ref_n(f, 1 + childCount);
    lean_inc(ctx->value);
    lean_apply_1(f, ctx->value);
    for (size_t i = 0; i < childCount; ++i) {
        lean_inc_ref(ctx->children[i]);
        lean_apply_1(f, ctx->children[i]);
    }
}

static void lean_yoga_Config_foreach(void* cfg, b_lean_obj_arg f) {
    lean_yoga_Config_context* ctx = YGConfigGetContext(*(YGConfigRef*)cfg);
    lean_inc_ref(f);
    lean_inc(ctx->value);
    lean_apply_1(f, ctx->value);
}

static void lean_yoga_Node_finalizer(void* node) {
    lean_yoga_Node_context* ctx = YGNodeGetContext((YGNodeRef)node);
    lean_dec(ctx->value);
    lean_dec_ref(ctx->config);
    size_t childCount = YGNodeGetChildCount((YGNodeRef)node);
    for (size_t i = 0; i < childCount; ++i) {
        lean_dec_ref(ctx->children[i]);
    }
    free(ctx->children);
    lean_yoga_free(ctx);
    YGNodeFree((YGNodeRef)node);
}

static void lean_yoga_Config_finalizer(void* cfg) {
    lean_yoga_Config_context* ctx = YGConfigGetContext((YGConfigRef)cfg);
    lean_dec(ctx->value);
    lean_yoga_free(ctx);
    YGConfigFree((YGConfigRef)cfg);
}

LEAN_EXPORT lean_obj_res lean_yoga_initialize(lean_obj_arg world) {
    lean_yoga_Node_class = lean_register_external_class(lean_yoga_Node_finalizer, lean_yoga_Node_foreach);
    lean_yoga_Config_class = lean_register_external_class(lean_yoga_Config_finalizer, lean_yoga_Config_foreach);
    return lean_io_result_mk_ok(lean_box(0));
}

static inline lean_object* lean_yoga_Node_box(YGNodeRef ref, lean_yoga_Node_context ctx) {
    lean_yoga_Node_context* ctxBoxed = lean_yoga_alloc(sizeof(lean_yoga_Node_context));
    *ctxBoxed = ctx;
    YGNodeSetContext(ref, ctxBoxed);
    return lean_alloc_external(lean_yoga_Node_class, ref);
}

static inline YGNodeRef lean_yoga_Node_unbox(lean_object* node) {
    return (YGNodeRef)lean_get_external_data(node);
}

static inline lean_object* lean_yoga_Config_box(YGConfigRef ref, lean_yoga_Config_context ctx) {
    lean_yoga_Config_context* ctxBoxed = lean_yoga_alloc(sizeof(lean_yoga_Config_context));
    *ctxBoxed = ctx;
    YGConfigSetContext(ref, ctxBoxed);
    return lean_alloc_external(lean_yoga_Config_class, ref);
}

static inline YGConfigRef lean_yoga_Config_unbox(lean_object* cfg) {
    return (YGConfigRef)lean_get_external_data(cfg);
}

static inline lean_object* lean_yoga_Value_box(YGValue value) {
    lean_object* ctor = lean_alloc_ctor(0, 2, 0);
    lean_ctor_set(ctor, 0, lean_pod_Float32_box(value.value));
    lean_ctor_set(ctor, 1, lean_box(value.unit));
    return ctor;
}

LEAN_EXPORT uint8_t lean_yoga_Node_beq(b_lean_obj_arg node1, b_lean_obj_arg node2) {
    return lean_yoga_Node_unbox(node1) == lean_yoga_Node_unbox(node2);
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_new(lean_obj_arg ctxVal, lean_obj_arg cfgCtxVal, lean_obj_arg world) {
    YGConfigRef cfg = YGConfigNew();
    lean_yoga_Config_context cfgCtx = {
        .value = cfgCtxVal,
    };
    lean_yoga_Node_context ctx = {
        .value = ctxVal,
        .config = lean_yoga_Config_box(cfg, cfgCtx),
        .parent = NULL,
        .children = NULL,
        .childrenCapacity = 0
    };
    YGNodeRef node = YGNodeNewWithConfig(cfg);
    return lean_io_result_mk_ok(lean_yoga_Node_box(node, ctx));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_getContext(b_lean_obj_arg cfg, lean_obj_arg world) {
    lean_yoga_Config_context* ctx = YGConfigGetContext(lean_yoga_Config_unbox(cfg));
    lean_inc(ctx->value);
    return lean_io_result_mk_ok(ctx->value);
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setContext(b_lean_obj_arg cfg, lean_obj_arg ctxVal, lean_obj_arg world) {
    lean_yoga_Config_context* ctx = YGConfigGetContext(lean_yoga_Config_unbox(cfg));
    lean_dec(ctx->value);
    ctx->value = ctxVal;
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_newWithConfig(lean_obj_arg ctxVal, lean_obj_arg cfg, lean_obj_arg world) {
    YGConfigRef ygCfg = lean_yoga_Config_unbox(cfg);
    YGNodeRef node = YGNodeNewWithConfig(ygCfg);
    lean_yoga_Node_context ctx = {
        .value = ctxVal,
        .config = cfg,
        .parent = NULL,
        .children = NULL,
        .childrenCapacity = 0
    };
    return lean_io_result_mk_ok(lean_yoga_Node_box(node, ctx));
}

// LEAN_EXPORT lean_obj_res lean_yoga_Node_clone(b_lean_obj_arg node, lean_obj_arg world) {
//     YGNodeRef nodeClone = YGNodeClone(lean_yoga_Node_unbox(node));
//     // todo: inc config, children ??
//     return lean_io_result_mk_ok(lean_yoga_Node_box(nodeClone, ctx));
// }

LEAN_EXPORT lean_obj_res lean_yoga_Node_reset(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* ctx = YGNodeGetContext(ygNode);
    YGNodeReset(ygNode); // keeps config
    YGNodeSetContext(ygNode, ctx);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_insertChild(
    b_lean_obj_arg node, lean_obj_arg child, uint32_t index, lean_obj_arg world
) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* nodeCtx = YGNodeGetContext(ygNode);
    YGNodeRef ygChild = lean_yoga_Node_unbox(child);
    lean_yoga_Node_context* childCtx = YGNodeGetContext(ygChild);
    if (YGNodeHasMeasureFunc(ygNode)) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string(
            "Yoga Node.insertChild: parent has a measure function"
        )));
    }
    if (childCtx->parent != NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string(
            "Yoga Node.insertChild: child already has a parent"
        )));
    }
    size_t childCount = YGNodeGetChildCount(ygNode);
    if (index >= childCount) {
        if (nodeCtx->children == NULL) {
            nodeCtx->children = malloc(1 * sizeof(lean_object*));
            nodeCtx->childrenCapacity = 1;
        }
        else if (nodeCtx->childrenCapacity < childCount + 1) {
            nodeCtx->childrenCapacity = 2 * childCount + 1;
            lean_object** children = malloc(nodeCtx->childrenCapacity * sizeof(lean_object*));
            memcpy(children, nodeCtx->children, childCount);
            free(nodeCtx->children);
            nodeCtx->children = children;
        }
        nodeCtx->children[childCount] = child;
    }
    else {
        if (nodeCtx->childrenCapacity < childCount + 1) {
            nodeCtx->childrenCapacity = 2 * childCount + 1;
            lean_object** children = malloc(nodeCtx->childrenCapacity * sizeof(lean_object*));
            memcpy(children, nodeCtx->children, index);
            children[index] = child;
            memcpy(children + index + 1, nodeCtx->children + index, childCount - index);
            free(nodeCtx->children);
            nodeCtx->children = children;
        }
        else {
            memmove(nodeCtx->children + index + 1, nodeCtx->children + index, childCount - index);
            nodeCtx->children[index] = child;
        }
    }
    YGNodeInsertChild(ygNode, ygChild, index);
    childCtx->parent = node;
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_swapChild(
    b_lean_obj_arg node, b_lean_obj_arg child, uint32_t index, lean_obj_arg world
) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* nodeCtx = YGNodeGetContext(ygNode);
    YGNodeRef ygChild = lean_yoga_Node_unbox(child);
    lean_yoga_Node_context* childCtx = YGNodeGetContext(ygChild);
    size_t childCount = YGNodeGetChildCount(ygNode);
    if (index >= childCount) {
        return lean_io_result_mk_ok(lean_box(0));
    }
    lean_object* otherChild = nodeCtx->children[index];
    ((lean_yoga_Node_context*)YGNodeGetContext(lean_yoga_Node_unbox(otherChild)))->parent = NULL;
    lean_dec_ref(otherChild);
    YGNodeSwapChild(ygNode, ygChild, index);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_removeChild(b_lean_obj_arg node, b_lean_obj_arg child, lean_obj_arg world) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* nodeCtx = YGNodeGetContext(ygNode);
    YGNodeRef ygChild = lean_yoga_Node_unbox(child);
    lean_yoga_Node_context* childCtx = YGNodeGetContext(ygChild);
    size_t childCount = YGNodeGetChildCount(ygNode);
    for (size_t i = 0; i < childCount; ++i) {
        if (nodeCtx->children[i] == child) {
            lean_dec_ref(nodeCtx->children[i]);
            childCtx->parent = NULL;
            memmove(nodeCtx->children + i, nodeCtx->children + i + 1, childCount - i - 1);
            YGNodeRemoveChild(ygNode, ygChild);
            break;
        }
    }
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_removeAllChildren(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* ctx = YGNodeGetContext(ygNode);
    for(size_t i = 0; i < YGNodeGetChildCount(ygNode); ++i) {
        YGNodeRef ygChild = lean_yoga_Node_unbox(ctx->children[i]);
        ((lean_yoga_Node_context*)YGNodeGetContext(ygChild))->parent = NULL;
        lean_dec_ref(ctx->children[i]);
    }
    free(ctx->children);
    ctx->children = NULL;
    YGNodeRemoveAllChildren(ygNode);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getChild(b_lean_obj_arg node, uint32_t i, lean_obj_arg world) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* ctx = YGNodeGetContext(ygNode);
    if (i >= YGNodeGetChildCount(ygNode)) {
        return lean_io_result_mk_ok(lean_box(0));
    }
    lean_inc_ref(ctx->children[i]);
    lean_object* option = lean_alloc_ctor(1, 1, 0);
    lean_ctor_set(option, 0, ctx->children[i]);
    return lean_io_result_mk_ok(option);
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getParent(b_lean_obj_arg node, lean_obj_arg world) {
    lean_yoga_Node_context* ctx = YGNodeGetContext(lean_yoga_Node_unbox(node));
    if (ctx->parent == NULL) {
        return lean_io_result_mk_ok(lean_box(0));
    }
    lean_inc_ref(ctx->parent);
    lean_object* option = lean_alloc_ctor(1, 1, 0);
    lean_ctor_set(option, 0, ctx->parent);
    return lean_io_result_mk_ok(option);
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getChildCount(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box_uint32(YGNodeGetChildCount(lean_yoga_Node_unbox(node))));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setChildren(
    b_lean_obj_arg node, b_lean_obj_arg children, lean_obj_arg world
) {
    YGNodeRef ygNode = lean_yoga_Node_unbox(node);
    lean_yoga_Node_context* nodeCtx = YGNodeGetContext(ygNode);
    if (YGNodeHasMeasureFunc(ygNode)) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string(
            "Yoga Node.setChildren: parent has a measure function"
        )));
    }
    size_t childCount = YGNodeGetChildCount(ygNode);
    size_t newChildCount = lean_array_size(children);
    YGNodeRef* ygChildren = malloc(newChildCount * sizeof(YGNodeRef));
    if (childCount == 0) {
        nodeCtx->children = malloc(newChildCount * sizeof(lean_object*));
        nodeCtx->childrenCapacity = newChildCount;
        for (size_t i = 0; i < newChildCount; ++i) {
            lean_object* child = lean_array_get_core(children, i);
            if (((lean_yoga_Node_context*)YGNodeGetContext(lean_yoga_Node_unbox(child)))->parent != NULL) {
                free(ygChildren);
                for (size_t j = 0; j < i; ++j) {
                    lean_dec_ref(nodeCtx->children[j]);
                    nodeCtx->children[j] = NULL;
                }
                return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string(
                    "Yoga Node.setChildren: a child already has a parent"
                )));
            }
            lean_inc_ref(child);
            nodeCtx->children[i] = child;
            ygChildren[i] = lean_yoga_Node_unbox(child);
        }
    }
    else {
        for (size_t i = 0; i < childCount; ++i) {
            ((lean_yoga_Node_context*)YGNodeGetContext(lean_yoga_Node_unbox(nodeCtx->children[i])))->parent = NULL;
            lean_dec_ref(nodeCtx->children[i]);
        }
        if (nodeCtx->childrenCapacity < newChildCount) {
            nodeCtx->childrenCapacity = newChildCount;
            free(nodeCtx->children);
            nodeCtx->children = malloc(newChildCount * sizeof(lean_object**));
        }
        for (size_t i = 0; i < newChildCount; ++i) {
            lean_object* child = lean_array_get_core(children, i);
            lean_inc_ref(child);
            nodeCtx->children[i] = child;
            ygChildren[i] = lean_yoga_Node_unbox(child);
        }
    }
    YGNodeSetChildren(ygNode, ygChildren, newChildCount);
    free(ygChildren);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setIsReferenceBaseline(
    b_lean_obj_arg node, uint8_t isRefBaseline, lean_obj_arg world
) {
    YGNodeSetIsReferenceBaseline(lean_yoga_Node_unbox(node), isRefBaseline);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_isReferenceBaseline(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(YGNodeIsReferenceBaseline(lean_yoga_Node_unbox(node))));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_calculateLayout(
    b_lean_obj_arg node, uint32_t avWidth, uint32_t avHeight, uint8_t ownerDir, lean_obj_arg world
) {
    YGNodeCalculateLayout(
        lean_yoga_Node_unbox(node),
        lean_pod_Float32_fromBits(avWidth),
        lean_pod_Float32_fromBits(avHeight),
        ownerDir
    );
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_markDirty(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeMarkDirty(lean_yoga_Node_unbox(node));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_markDirtyAndPropagateToDescendants(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeMarkDirtyAndPropagateToDescendants(lean_yoga_Node_unbox(node));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_print(b_lean_obj_arg node, uint32_t opts, lean_obj_arg world) {
    YGNodePrint(lean_yoga_Node_unbox(node), (int32_t)opts);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT uint8_t lean_yoga_floatIsUndefined(uint32_t value) {
    return YGFloatIsUndefined(lean_pod_Float32_fromBits(value));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_canUseCachedMeasurement(
    uint8_t widthMode, uint32_t width, uint8_t heightMode, uint32_t height,
    uint8_t lastWidthMode, uint32_t lastWidth, uint8_t lastHeightMode, uint32_t lastHeight,
    uint32_t lastComputedWidth, uint32_t lastComputedHeight,
    uint32_t marginRow, uint32_t marginColumn,
    b_lean_obj_arg cfg, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_box(YGNodeCanUseCachedMeasurement(
        widthMode, lean_pod_Float32_fromBits(width),
        heightMode, lean_pod_Float32_fromBits(height),
        lastWidthMode, lean_pod_Float32_fromBits(lastWidth),
        lastHeightMode, lean_pod_Float32_fromBits(lastHeight),
        lean_pod_Float32_fromBits(lastComputedWidth), lean_pod_Float32_fromBits(lastComputedHeight),
        lean_pod_Float32_fromBits(marginRow), lean_pod_Float32_fromBits(marginColumn),
        lean_yoga_Config_unbox(cfg)
    )));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_copyStyle(b_lean_obj_arg dst, b_lean_obj_arg src, lean_obj_arg world) {
    YGNodeCopyStyle(lean_yoga_Node_unbox(dst), lean_yoga_Node_unbox(src));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getContext(b_lean_obj_arg node, lean_obj_arg world) {
    lean_yoga_Node_context* ctx = YGNodeGetContext(lean_yoga_Node_unbox(node));
    lean_inc(ctx->value);
    return lean_io_result_mk_ok(ctx->value);
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setContext(b_lean_obj_arg node, lean_obj_arg ctxVal, lean_obj_arg world) {
    lean_yoga_Node_context* ctx = YGNodeGetContext(lean_yoga_Node_unbox(node));
    lean_dec(ctx->value);
    ctx->value = ctxVal;
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getConfig(b_lean_obj_arg node, lean_obj_arg world) {
    lean_yoga_Node_context* ctx = YGNodeGetContext(lean_yoga_Node_unbox(node));
    lean_inc_ref(ctx->config);
    return lean_io_result_mk_ok(ctx->config);
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setConfig(b_lean_obj_arg node, lean_obj_arg cfg, lean_obj_arg world) {
    lean_yoga_Node_context* ctx = YGNodeGetContext(lean_yoga_Node_unbox(node));
    lean_dec_ref(ctx->config);
    ctx->config = cfg;
    YGNodeSetConfig(lean_yoga_Node_unbox(node), lean_yoga_Config_unbox(cfg));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setPrintTreeFlag(
    b_lean_obj_arg cfg, uint8_t enabled, lean_obj_arg world
) {
    YGConfigSetPrintTreeFlag(lean_yoga_Config_unbox(cfg), enabled);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_hasMeasureFunc(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeHasMeasureFunc(lean_yoga_Node_unbox(node))
    ));
}

// @[extern "lean_yoga_Node_setMeasureFunc"]
// opaque Node.setMeasureFunc (node : @& Node α β) (measureFunc : MeasureFunc α β) : IO Unit

LEAN_EXPORT lean_obj_res lean_yoga_Node_hasBaselineFunc(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeHasBaselineFunc(lean_yoga_Node_unbox(node))
    ));
}

// @[extern "lean_yoga_Node_setBaselineFunc"]
// opaque Node.setBaselineFunc (node : @& Node α β) (baselineFunc : BaselineFunc α β) : IO Unit

// @[extern "lean_yoga_Node_getDirtiedFunc"]
// opaque Node.getDirtiedFunc (node : @& Node α β) : IO (DirtiedFunc α β)

// @[extern "lean_yoga_Node_setDirtiedFunc"]
// opaque Node.setDirtiedFunc (node : @& Node α β) (dirtiedFunc : DirtiedFunc α β) : IO Unit

// @[extern "lean_yoga_Node_getHasNewLayout"]
// opaque Node.getHasNewLayout (node : @& Node α β) : IO Bool

LEAN_EXPORT lean_obj_res lean_yoga_Node_getHasNewLayout(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeGetHasNewLayout(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setHasNewLayout(
    b_lean_obj_arg node, uint8_t hasNewLayout, lean_obj_arg world
) {
    YGNodeSetHasNewLayout(lean_yoga_Node_unbox(node), hasNewLayout);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_getNodeType(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeGetNodeType(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_setNodeType(
    b_lean_obj_arg node, uint8_t nodeType, lean_obj_arg world
) {
    YGNodeSetNodeType(lean_yoga_Node_unbox(node), nodeType);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_isDirty(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeIsDirty(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetDirection(
    b_lean_obj_arg node, uint8_t direction, lean_obj_arg world
) {
    YGNodeStyleSetDirection(lean_yoga_Node_unbox(node), direction);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetDirection(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetDirection(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexDirection(
    b_lean_obj_arg node, uint8_t flexDirection, lean_obj_arg world
) {
    YGNodeStyleSetFlexDirection(lean_yoga_Node_unbox(node), flexDirection);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlexDirection(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetFlexDirection(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetJustifyContent(
    b_lean_obj_arg node, uint8_t justifyContent, lean_obj_arg world
) {
    YGNodeStyleSetJustifyContent(lean_yoga_Node_unbox(node), justifyContent);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetJustifyContent(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetJustifyContent(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetAlignContent(
    b_lean_obj_arg node, uint8_t alignContent, lean_obj_arg world
) {
    YGNodeStyleSetAlignContent(lean_yoga_Node_unbox(node), alignContent);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetAlignContent(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetAlignContent(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetAlignItems(
    b_lean_obj_arg node, uint8_t alignItems, lean_obj_arg world
) {
    YGNodeStyleSetAlignItems(lean_yoga_Node_unbox(node), alignItems);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetAlignItems(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetAlignItems(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetAlignSelf(
    b_lean_obj_arg node, uint8_t alignSelf, lean_obj_arg world
) {
    YGNodeStyleSetAlignSelf(lean_yoga_Node_unbox(node), alignSelf);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetAlignSelf(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetAlignSelf(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetPositionType(
    b_lean_obj_arg node, uint8_t positionType, lean_obj_arg world
) {
    YGNodeStyleSetPositionType(lean_yoga_Node_unbox(node), positionType);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetPositionType(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetPositionType(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexWrap(
    b_lean_obj_arg node, uint8_t flexWrap, lean_obj_arg world
) {
    YGNodeStyleSetFlexWrap(lean_yoga_Node_unbox(node), flexWrap);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlexWrap(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetFlexWrap(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetOverflow(
    b_lean_obj_arg node, uint8_t overflow, lean_obj_arg world
) {
    YGNodeStyleSetOverflow(lean_yoga_Node_unbox(node), overflow);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetOverflow(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetOverflow(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetDisplay(
    b_lean_obj_arg node, uint8_t display, lean_obj_arg world
) {
    YGNodeStyleSetDisplay(lean_yoga_Node_unbox(node), display);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetDisplay(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeStyleGetDisplay(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlex(
    b_lean_obj_arg node, uint32_t flex, lean_obj_arg world
) {
    YGNodeStyleSetFlex(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(flex));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlex(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetFlex(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexGrow(
    b_lean_obj_arg node, uint32_t flexGrow, lean_obj_arg world
) {
    YGNodeStyleSetFlexGrow(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(flexGrow));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlexGrow(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetFlexGrow(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexShrink(
    b_lean_obj_arg node, uint32_t flexShrink, lean_obj_arg world
) {
    YGNodeStyleSetFlexShrink(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(flexShrink));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlexShrink(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetFlexShrink(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexBasis(
    b_lean_obj_arg node, uint32_t flexBasis, lean_obj_arg world
) {
    YGNodeStyleSetFlexBasis(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(flexBasis));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexBasisPercent(
    b_lean_obj_arg node, uint32_t flexBasis, lean_obj_arg world
) {
    YGNodeStyleSetFlexBasisPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(flexBasis));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetFlexBasisAuto(
    b_lean_obj_arg node, lean_obj_arg world
) {
    YGNodeStyleSetFlexBasisAuto(lean_yoga_Node_unbox(node));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetFlexBasis(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetFlexBasis(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetPosition(
    b_lean_obj_arg node, uint8_t edge, uint32_t position, lean_obj_arg world
) {
    YGNodeStyleSetPosition(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(position));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetPositionPercent(
    b_lean_obj_arg node, uint8_t edge, uint32_t position, lean_obj_arg world
) {
    YGNodeStyleSetPositionPercent(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(position));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetPosition(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetPosition(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMargin(
    b_lean_obj_arg node, uint8_t edge, uint32_t margin, lean_obj_arg world
) {
    YGNodeStyleSetMargin(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(margin));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMarginPercent(
    b_lean_obj_arg node, uint8_t edge, uint32_t margin, lean_obj_arg world
) {
    YGNodeStyleSetMarginPercent(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(margin));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMarginAuto(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    YGNodeStyleSetMarginAuto(lean_yoga_Node_unbox(node), edge);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetMargin(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetMargin(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetPadding(
    b_lean_obj_arg node, uint8_t edge, uint32_t padding, lean_obj_arg world
) {
    YGNodeStyleSetPadding(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(padding));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetPaddingPercent(
    b_lean_obj_arg node, uint8_t edge, uint32_t padding, lean_obj_arg world
) {
    YGNodeStyleSetPaddingPercent(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(padding));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetPadding(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetPadding(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetBorder(
    b_lean_obj_arg node, uint8_t edge, uint32_t border, lean_obj_arg world
) {
    YGNodeStyleSetGap(lean_yoga_Node_unbox(node), edge, lean_pod_Float32_fromBits(border));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetBorder(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetBorder(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetGap(
    b_lean_obj_arg node, uint8_t gutter, uint32_t gapLength, lean_obj_arg world
) {
    YGNodeStyleSetGap(lean_yoga_Node_unbox(node), gutter, lean_pod_Float32_fromBits(gapLength));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetGap(
    b_lean_obj_arg node, uint8_t gutter, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetGap(lean_yoga_Node_unbox(node), gutter)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetWidth(
    b_lean_obj_arg node, uint32_t width, lean_obj_arg world
) {
    YGNodeStyleSetWidth(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(width));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetWidthPercent(
    b_lean_obj_arg node, uint32_t width, lean_obj_arg world
) {
    YGNodeStyleSetWidthPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(width));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetWidthAuto(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeStyleSetWidthAuto(lean_yoga_Node_unbox(node));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetWidth(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetWidth(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetHeight(
    b_lean_obj_arg node, uint32_t height, lean_obj_arg world
) {
    YGNodeStyleSetHeight(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(height));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetHeightPercent(
    b_lean_obj_arg node, uint32_t height, lean_obj_arg world
) {
    YGNodeStyleSetHeightPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(height));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetHeightAuto(b_lean_obj_arg node, lean_obj_arg world) {
    YGNodeStyleSetHeightAuto(lean_yoga_Node_unbox(node));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetHeight(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetHeight(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMinWidth(
    b_lean_obj_arg node, uint32_t minWidth, lean_obj_arg world
) {
    YGNodeStyleSetMinWidth(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(minWidth));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMinWidthPercent(
    b_lean_obj_arg node, uint32_t minWidth, lean_obj_arg world
) {
    YGNodeStyleSetMinWidthPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(minWidth));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetMinWidth(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetMinWidth(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMinHeight(
    b_lean_obj_arg node, uint32_t minHeight, lean_obj_arg world
) {
    YGNodeStyleSetMinHeight(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(minHeight));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMinHeightPercent(
    b_lean_obj_arg node, uint32_t minHeight, lean_obj_arg world
) {
    YGNodeStyleSetMinHeightPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(minHeight));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetMinHeight(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetMinHeight(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMaxWidth(
    b_lean_obj_arg node, uint32_t maxWidth, lean_obj_arg world
) {
    YGNodeStyleSetMaxWidth(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(maxWidth));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMaxWidthPercent(
    b_lean_obj_arg node, uint32_t maxWidth, lean_obj_arg world
) {
    YGNodeStyleSetMaxWidthPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(maxWidth));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetMaxWidth(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetMaxWidth(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMaxHeight(
    b_lean_obj_arg node, uint32_t maxHeight, lean_obj_arg world
) {
    YGNodeStyleSetMaxHeight(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(maxHeight));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetMaxHeightPercent(
    b_lean_obj_arg node, uint32_t maxHeight, lean_obj_arg world
) {
    YGNodeStyleSetMaxHeightPercent(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(maxHeight));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetMaxHeight(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_yoga_Value_box(
        YGNodeStyleGetMaxHeight(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleSetAspectRatio(
    b_lean_obj_arg node, uint32_t aspectRatio, lean_obj_arg world
) {
    YGNodeStyleSetAspectRatio(lean_yoga_Node_unbox(node), lean_pod_Float32_fromBits(aspectRatio));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_styleGetAspectRatio(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeStyleGetAspectRatio(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetLeft(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetLeft(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetTop(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetTop(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetRight(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetRight(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetBottom(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetBottom(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetWidth(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetWidth(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetHeight(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetHeight(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetDirection(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeLayoutGetDirection(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetHadOverflow(b_lean_obj_arg node, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGNodeLayoutGetHadOverflow(lean_yoga_Node_unbox(node))
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetMargin(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetMargin(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetBorder(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetBorder(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_Node_layoutGetPadding(
    b_lean_obj_arg node, uint8_t edge, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGNodeLayoutGetPadding(lean_yoga_Node_unbox(node), edge)
    ));
}

LEAN_EXPORT lean_obj_res lean_yoga_assert(uint8_t cond, b_lean_obj_arg msg, lean_obj_arg world) {
    YGAssert(cond, lean_string_cstr(msg));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_assertWithNode(b_lean_obj_arg node, uint8_t cond, b_lean_obj_arg msg, lean_obj_arg world) {
    YGAssertWithNode(lean_yoga_Node_unbox(node), cond, lean_string_cstr(msg));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_assertWithConfig(
    b_lean_obj_arg cfg, uint8_t cond, b_lean_obj_arg msg, lean_obj_arg world
) {
    YGAssertWithConfig(lean_yoga_Config_unbox(cfg), cond, lean_string_cstr(msg));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setPointScaleFactor(
    b_lean_obj_arg cfg, uint32_t pixelsInPoint, lean_obj_arg world
) {
    YGConfigSetPointScaleFactor(lean_yoga_Config_unbox(cfg), lean_pod_Float32_fromBits(pixelsInPoint));
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_getPointScaleFactor(b_lean_obj_arg cfg, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_pod_Float32_box(
        YGConfigGetPointScaleFactor(lean_yoga_Config_unbox(cfg))
    ));
}

LEAN_EXPORT uint8_t lean_yoga_Config_beq(b_lean_obj_arg cfg1, b_lean_obj_arg cfg2) {
    return lean_yoga_Config_unbox(cfg1) == lean_yoga_Config_unbox(cfg2);
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_new(lean_obj_arg ctxVal, lean_obj_arg world) {
    YGConfigRef cfg = YGConfigNew();
    lean_yoga_Config_context ctx = { .value = ctxVal };
    return lean_io_result_mk_ok(lean_yoga_Config_box(cfg, ctx));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_copy(b_lean_obj_arg dst, b_lean_obj_arg src, lean_obj_arg world) {
    YGConfigRef dstCfg = lean_yoga_Config_unbox(dst);
    YGConfigRef srcCfg = lean_yoga_Config_unbox(src);
    lean_yoga_Node_context* dstCtx = YGConfigGetContext(dstCfg);
    lean_yoga_Node_context* srcCtx = YGConfigGetContext(srcCfg);
    YGConfigCopy(dstCfg, srcCfg);
    YGConfigSetContext(dstCfg, dstCtx);
    lean_dec(dstCtx->value);
    lean_inc(srcCtx->value);
    dstCtx->value = srcCtx->value;
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_getInstanceCount(lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box_uint32(YGConfigGetInstanceCount()));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setExperimentalFeatureEnabled(
    b_lean_obj_arg cfg, uint8_t feature, uint8_t enabled, lean_obj_arg world
) {
    YGConfigSetExperimentalFeatureEnabled(lean_yoga_Config_unbox(cfg), feature, enabled);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_isExperimentalFeatureEnabled(
    b_lean_obj_arg cfg, uint8_t feature, lean_obj_arg world
) {
    return lean_io_result_mk_ok(lean_box(YGConfigIsExperimentalFeatureEnabled(
        lean_yoga_Config_unbox(cfg), feature
    )));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setUseWebDefaults(
    b_lean_obj_arg cfg, uint8_t enabled, lean_obj_arg world
) {
    YGConfigSetUseWebDefaults(lean_yoga_Config_unbox(cfg), enabled);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_getUseWebDefaults(b_lean_obj_arg cfg, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box(
        YGConfigGetUseWebDefaults(lean_yoga_Config_unbox(cfg))
    ));
}

// LEAN_EXPORT lean_obj_res lean_yoga_Config_setCloneNodeFunc(
//     b_lean_obj_arg cfg, lean_obj_arg cb, lean_obj_arg world
// ) {
//     YGConfigSetCloneNodeFunc()
// }

LEAN_EXPORT lean_obj_res lean_yoga_Config_getDefault(lean_obj_arg ctxVal, lean_obj_arg world) {
    YGConfigRef cfg = YGConfigNew();
    YGConfigCopy(cfg, YGConfigGetDefault());
    lean_yoga_Config_context ctx = { .value = ctxVal };
    return lean_io_result_mk_ok(lean_yoga_Config_box(cfg, ctx));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_setErrata(
    b_lean_obj_arg cfg, uint32_t errata, lean_obj_arg world
) {
    YGConfigSetErrata(lean_yoga_Config_unbox(cfg), (int32_t)errata);
    return lean_io_result_mk_ok(lean_box(0));
}

LEAN_EXPORT lean_obj_res lean_yoga_Config_getErrata(b_lean_obj_arg cfg, lean_obj_arg world) {
    return lean_io_result_mk_ok(lean_box_uint32(
        (int32_t)YGConfigGetErrata(lean_yoga_Config_unbox(cfg))
    ));
}

LEAN_EXPORT uint32_t lean_yoga_roundValueToPixelGrid(
    double value, double pointScaleFactor, uint8_t forceCeil, uint8_t forceFloor
) {
    return lean_pod_Float32_toBits(
        YGRoundValueToPixelGrid(value, pointScaleFactor, forceCeil, forceFloor)
    );
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
