import Lake
open Lake DSL

def packagesDir := defaultPackagesDir
def yogaVersion := "v2.0.0"

def podConfig : NameMap String := Id.run $ do
  let mut cfg := NameMap.empty
  if let some cc := get_config? cc then
    cfg := cfg.insert `cc cc
  if let some alloc := get_config? alloc then
    cfg := cfg.insert `alloc alloc
  cfg

require pod from git "https://github.com/KislyjKisel/lean-pod" @ "main" with podConfig

package «yoga» {
  srcDir := "src/lean"
  packagesDir := packagesDir
  moreLeanArgs := #["-DautoImplicit=false"]
  moreServerArgs := #["-DautoImplicit=false"]
  moreLinkArgs := #[s!"-L{__dir__}/yoga/yoga/build", "-lyogacore"]
}

lean_lib Yoga

@[default_target]
lean_exe Test

def buildBindingsO (pkg : Package) (flags : Array String) (stem : String) : IndexBuildM (BuildJob FilePath) := do
  let oFile := pkg.irDir / "native" / (stem ++ ".o")
  let srcJob ← inputFile <| pkg.dir / "src" / "native" / (stem ++ ".c")
  buildO (stem ++ ".c") oFile srcJob flags ((get_config? cc).getD (← getLeanCc).toString)

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
    args := #["-DCMAKE_BUILD_TYPE=Release", ".."]
    cwd := __dir__ / "yoga" / "yoga" / "build"
  }
  if printCmdOutput then IO.println cmakeOutput

  let cmakeBuildOutput ← tryRunProcess {
    cmd := "cmake"
    args := #["--build", "."]
    cwd := __dir__ / "yoga" / "yoga" / "build"
  }
  if printCmdOutput then IO.println cmakeBuildOutput

def bindingsCFlags (pkg : NPackage _package.name) : IndexBuildM (Array String) := do
  let mut flags := #["-I", (← getLeanIncludeDir).toString, "-fPIC"]

  match pkg.deps.find? λ dep ↦ dep.name == `pod with
  | none => error "Missing dependency 'Pod'"
  | some pod =>
    flags := flags ++ #[
      "-I",
      (pod.dir / "src" / "native" / "include").toString
    ]

  if !(← (__dir__ / "yoga" / "yoga" / "build" / nameToStaticLib "yogacore").pathExists) then
    buildYogaSubmodule true
  flags := flags.append #[
    "-I",
    (pkg.dir / "yoga").toString
  ]

  match get_config? alloc with
  | .none | .some "lean" => pure ()
  | .some "native" => flags := flags.push "-DLEAN_YOGA_ALLOC_NATIVE"
  | .some _ => error "Unknown `alloc` option value"

  if (get_config? cc).isNone then
    flags := flags ++ #["-I", ((← getLeanIncludeDir) / "clang").toString]

  if (get_config? skipTests).isSome then
    flags := flags.push "-DLEAN_YOGA_SKIP_TESTS"

  pure flags

extern_lib «yoga-lean» pkg := do
  let name := nameToStaticLib "yoga-lean"
  let flags ← bindingsCFlags pkg
  let bindingsOFile ← buildBindingsO pkg flags "ffi"
  buildStaticLib (pkg.nativeLibDir / name) #[bindingsOFile]

script buildYoga do
  buildYogaSubmodule true
  return 0
