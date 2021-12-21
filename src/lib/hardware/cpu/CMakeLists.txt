target_sources(wm-sensors PRIVATE
cpuid.cxx
cpuid.hxx
cpu_load.cxx
cpu_load.hxx
probe.cxx
probe.hxx
generic_cpu.cxx
generic_cpu.hxx
amd/amd_cpu.cxx
amd/amd_cpu.hxx
amd/amd0f_cpu.cxx
amd/amd0f_cpu.hxx
amd/amd10_cpu.cxx
amd/amd10_cpu.hxx
amd/amd17_cpu.cxx
amd/amd17_cpu.hxx
amd/ryzen_smu.cxx
amd/ryzen_smu.hxx
intel/intel_cpu.cxx
intel/intel_cpu.hxx
)

target_link_libraries(wm-sensors PRIVATE PowrProf)
