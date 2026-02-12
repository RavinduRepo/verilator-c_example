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



