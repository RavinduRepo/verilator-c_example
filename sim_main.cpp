#include "Vcounter.h"
#include "verilated.h"
#include "verilated_vcd_c.h" // <--- CRITICAL: Include the VCD library

int main(int argc, char **argv) {
  // 1. Setup Context
  VerilatedContext *contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);

  // 2. Create Model
  Vcounter *top = new Vcounter{contextp};

  // 3. Setup Trace Dumper (Waveform Generation)
  Verilated::traceEverOn(true); // Enable tracing globally
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  top->trace(m_trace, 99);       // Trace 99 levels of hierarchy
  m_trace->open("waveform.vcd"); // Open the file to dump to

  // 4. Initialize Signals
  top->reset = 1;
  top->clk = 0;

  // 5. Simulation Loop
  while (!contextp->gotFinish() && contextp->time() < 50) {
    contextp->timeInc(1); // Advance time

    // Toggle Clock
    top->clk = !top->clk;

    // Release Reset
    if (contextp->time() > 4) {
      top->reset = 0;
    }

    // Evaluate Model
    top->eval();

    // Dump signals to VCD file (at every time step)
    m_trace->dump(contextp->time());

    // Print to Console (Optional debug)
    if (top->clk == 1) {
      printf("Time: %ld | Count: %d\n", contextp->time(), top->count);
    }
  }

  // 6. Final Cleanup
  m_trace->close(); // <--- CRITICAL: Close file to ensure data is written
  delete top;
  delete contextp;
  return 0;
}
