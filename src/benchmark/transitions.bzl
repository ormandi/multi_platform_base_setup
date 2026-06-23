"""Manylinux stdc++ transition and a generic cc_library wrapper.

The C++ standard library is selected purely through a target-platform
constraint, so this transition forces stdc++ by remapping --platforms from a
libc++ Linux platform to its GNU libstdc++ counterpart. It is gated by
//:manylinux_compatible, idempotent, and a no-op for macOS or
already-stdc++ platforms.
"""

load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

_LIBCXX_TO_STDCXX = {
    str(Label("//platforms:linux_arm64_libcxx")): Label("//platforms:linux_arm64_stdcxx"),
    str(Label("//platforms:linux_x86_64_libcxx")): Label("//platforms:linux_x86_64_stdcxx"),
}

def _manylinux_impl(settings, attr):
    if not settings["//:manylinux_compatible"]:
        return {}
    platforms = [
        _LIBCXX_TO_STDCXX.get(str(p), p)
        for p in settings["//command_line_option:platforms"]
    ]
    return {"//command_line_option:platforms": platforms}

manylinux_transition = transition(
    implementation = _manylinux_impl,
    inputs = [
        "//:manylinux_compatible",
        "//command_line_option:platforms",
    ],
    outputs = ["//command_line_option:platforms"],
)

def _manylinux_library_impl(ctx):
    dep = ctx.attr.actual[0]
    return [dep[CcInfo], dep[DefaultInfo]]

manylinux_library = rule(
    implementation = _manylinux_library_impl,
    doc = "Rebuilds a cc_library under the manylinux (stdc++) configuration.",
    attrs = {
        "actual": attr.label(
            mandatory = True,
            providers = [CcInfo],
            cfg = manylinux_transition,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
)
