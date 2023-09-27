import Lake
open Lake DSL

def packagesDir := defaultPackagesDir
def yogaVersion := "v2.0.0" -- todo: auto checkout ?
def cppStdlib := (get_config? cppStdlib).getD "c++"
def cppCompiler := (get_config? cppCompiler).getD "clang++"
def cCompiler := (get_config? cCompiler).getD "clang"

def podConfig : NameMap String := Id.run $ do
  let mut cfg := NameMap.empty
  if let some cc := get_config? cc then
    cfg := cfg.insert `cc cc
  if let some alloc := get_config? alloc then
    cfg := cfg.insert `alloc alloc
  cfg

-- Required for Float32, can be made standalone using a `def Float32 := UInt32`
require pod from git "https://github.com/KislyjKisel/lean-pod" @ "main" with podConfig

package «yoga» {
  srcDir := "src/lean"
  packagesDir := packagesDir
  moreLeanArgs := #["-DautoImplicit=false"]
  moreServerArgs := #["-DautoImplicit=false"]
  moreLinkArgs := #[
    s!"-L{__dir__}/yoga/yoga/build", "-lyogacore", s!"-l{cppStdlib}"
  ]
}

lean_lib Yoga

@[default_target]
lean_exe Test

def tryRunProcess {m} [Monad m] [MonadError m] [MonadLiftT IO m] (sa : IO.Process.SpawnArgs) : m String := do
  let output ← IO.Process.output sa
  if output.exitCode ≠ 0 then
    error s!"'{sa.cmd}' returned {output.exitCode}: {output.stderr}"
  else
    return output.stdout

def buildYogaSubmodule (printCmdOutput : Bool) : IO Unit := do
  let gitOutput ← tryRunProcess {
    cmd := "git"
    args := #["submodule", "update", "--init", "--force", "--recursive"]
    cwd := __dir__
  }
  if printCmdOutput then IO.println gitOutput

  let mkdirOutput ← tryRunProcess {
    cmd := "mkdir"
    args := #["-p", "yoga/yoga/build"]
    cwd := __dir__
  }
  if printCmdOutput then IO.println mkdirOutput

  let cmakeOutput ← tryRunProcess {
    cmd := "cmake"
    args := #[
      "-DCMAKE_BUILD_TYPE=Release",
      -- "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
      s!"-DCMAKE_C_COMPILER={cCompiler}",
      s!"-DCMAKE_CXX_COMPILER={cppCompiler}",
      s!"-DCMAKE_CXX_FLAGS=-stdlib=lib{cppStdlib}",
      ".."
    ]
    cwd := __dir__ / "yoga" / "yoga" / "build"
    env := #[("LD_LIBRARY_PATH", none)]
  }
  if printCmdOutput then IO.println cmakeOutput

  let cmakeBuildOutput ← tryRunProcess {
    cmd := "cmake"
    args := #["--build", "."]
    cwd := __dir__ / "yoga" / "yoga" / "build"
  }
  if printCmdOutput then IO.println cmakeBuildOutput

extern_lib «yoga-lean» pkg := do
  let name := nameToStaticLib "yoga-lean"

  let mut weakArgs := #["-I", (← getLeanIncludeDir).toString, "-I", (pkg.dir / "yoga").toString]
  let mut traceArgs := #["-fPIC"]
  match pkg.deps.find? λ dep ↦ dep.name == `pod with
  | none => error "Missing dependency 'Pod'"
  | some pod =>
    weakArgs := weakArgs ++ #[
      "-I",
      (pod.dir / "src" / "native" / "include").toString
    ]
  if !(← (__dir__ / "yoga" / "yoga" / "build" / nameToStaticLib "yogacore").pathExists) then
    buildYogaSubmodule false
  match get_config? alloc with
  | .none | .some "lean" => pure ()
  | .some "native" => traceArgs := traceArgs.push "-DLEAN_YOGA_ALLOC_NATIVE"
  | .some _ => error "Unknown `alloc` option value"
  if (get_config? skipTests).isSome then
    traceArgs := traceArgs.push "-DLEAN_YOGA_SKIP_TESTS"

  let cCompiler ←
    if cCompiler == "clang"
      then
        IO.eprintln $
          "Warning: `cCompiler=clang` may use Lean's bundled clang which won't work;" ++
          "falling back to `cc` to compile ffi."
        pure "cc"
      else
        pure cCompiler

  let oFile := pkg.irDir / "native" / "ffi.o"
  let fileName := "ffi.c"
  let srcJob ← inputFile <| pkg.dir / "src" / "native" / fileName
  let ffiO ← buildO ("native/" ++ fileName) oFile srcJob weakArgs traceArgs cCompiler
  buildStaticLib (pkg.nativeLibDir / name) #[ffiO]

script yogaBuild do
  buildYogaSubmodule true
  return 0

script yogaClean do
  if System.Platform.isWindows
    then
      let o1 ← tryRunProcess {
        cmd := "rmdir"
        args := #["/s", "/q", "yoga\\yoga\\build"]
      }
      IO.println o1
    else
      let o1 ← tryRunProcess {
        cmd := "rm"
        args := #["-rf", "yoga/yoga/build"]
      }
      IO.println o1
  return 0
