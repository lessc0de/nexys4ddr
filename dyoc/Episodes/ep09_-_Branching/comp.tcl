# This is a tcl command script for the Vivado tool chain
read_vhdl { \
   comp.vhd waiter.vhd clk.vhd main.vhd \
   vga/vga.vhd vga/digits.vhd vga/pix.vhd vga/font.vhd \
   cpu/cpu.vhd cpu/datapath.vhd cpu/ctl.vhd cpu/alu.vhd \
   mem/mem.vhd \
}
read_xdc comp.xdc
set_property XPM_LIBRARIES {XPM_CDC} [current_project]
synth_design -top comp -part xc7a100tcsg324-1 -flatten_hierarchy none
place_design
route_design
write_checkpoint -force comp.dcp
write_bitstream -force comp.bit
report_methodology
report_timing_summary -file timing_summary.rpt
exit
