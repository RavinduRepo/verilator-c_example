This is the complete, end-to-end guide.

This version includes the **C++ Waveform Tracing** logic integrated into your wrapper. This allows you to not just see text output, but to actually debug signals visually in GTKWave.

### Phase 1: Prerequisites

Since you want to view the waves, you need a viewer. On Arch:

```bash
sudo pacman -S gtkwave

```

---

### Phase 2: Create The Files

Create a new folder for this test to keep things clean:

```bash
mkdir -p ~/verilator_test
cd ~/verilator_test

```

#### 1. The Hardware (`counter.v`)

Save this as `counter.v`.

```systemverilog
module counter(
    input clk,
    input reset,
    output logic [7:0] count
);
    always_ff @(posedge clk) begin
        if (reset) count <= 0;
        else       count <= count + 1;
    end
endmodule

```

#### 2. The Simulation Wrapper (`sim_main.cpp`)

Save this as `sim_main.cpp`. I have added the **Trace Logic** comments clearly below.

```cpp
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
    Verilated::traceEverOn(true);         // Enable tracing globally
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    top->trace(m_trace, 99);              // Trace 99 levels of hierarchy
    m_trace->open("waveform.vcd");        // Open the file to dump to

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
    m_trace->close();  // <--- CRITICAL: Close file to ensure data is written
    delete top;
    delete contextp;
    return 0;
}

```

---

### Phase 3: The Build (Compiling)

You must now add the `--trace` flag. If you forget this, Verilator won't link the tracing libraries, and your C++ code will fail to compile.

Run this exact command:

```bash
verilator --cc --exe --build -j 0 --trace sim_main.cpp counter.v

```

* `--cc`: Convert Verilog to C++.
* `--exe`: Link the C++ wrapper (`sim_main.cpp`).
* `--build`: Automatically run `make` to compile the binary.
* `-j 0`: Use all CPU cores.
* `--trace`: **Enable waveform tracing support.**

---

### Phase 4: Run The Simulation

Execute the binary generated in `obj_dir`:

```bash
./obj_dir/Vcounter

```

**What just happened?**

1. **Console:** You should see the "Time: x | Count: y" prints in your terminal.
2. **Files:** A new file named `waveform.vcd` has been created in your current directory.

---

### Phase 5: View the Waveforms

Now, open the generated file with GTKWave:

```bash
gtkwave waveform.vcd

```

**How to use GTKWave:**

1. On the left panel (SST), click `TOP` -> `counter`.
2. You will see signals like `clk`, `reset`, `count` appear in the list below.
3. **Drag and drop** these signals into the black window ("Signals" area).
4. You will see the green waveforms appear! (You might need to click the "Zoom Fit" icon—the magnifying glass with a square inside it).

### Summary: The "Mental Model" of C++ Verilator

1. **Header:** include `verilated_vcd_c.h`.
2. **Setup:** Create `VerilatedVcdC` object and call `open()`.
3. **Loop:** Call `dump(time)` every cycle.
4. **Build:** Must use `--trace` flag.
