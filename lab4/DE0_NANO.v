`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

module DE0_NANO(
   CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY,
	RESULT

);





//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK //////////
input CLOCK_50;
wire PCLK = GPIO_1_D[32];
//wire HREF = GPIO_1_D[30];
wire HREF = GPIO_1_D[29];
wire VSYNC = GPIO_1_D[30];
//wire VSYNC = GPIO_1_D[31];

wire c0_sig;
wire c1_sig;
wire c2_sig;

sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ), //24
	.c1 ( c1_sig ), //25
	.c2 ( c2_sig )  //50 - phase synchronized with c0 and c1  
	);





//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);
assign GPIO_0_D[31] = RESULT[1];
assign GPIO_0_D[33] = RESULT[0];
assign GPIO_0_D[29] = 1'b1;
///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];
assign GPIO_0_D[1] = c0_sig;


///// I/O for Img Proc /////
output wire [1:0] RESULT;

/* WRITE ENABLE */
reg W_EN;


///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(c2_sig),
	.clk_R(c1_sig), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);


	


///////* Image Processor *///////

IMAGE_PROCESSOR proc (
	.PIXEL_IN (MEM_OUTPUT),
	.CLK(c1_sig),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


always @ (negedge HREF, posedge VSYNC) begin
	//if (Y_ADDR >= `SCREEN_HEIGHT - 1) begin
	//	Y_ADDR = 0;
	//end
	//else begin
	//	Y_ADDR = Y_ADDR + 1;
	//end
	if (VSYNC) begin
		Y_ADDR = 0;
	end
	else begin
		Y_ADDR = Y_ADDR+1;
	end
end


///////* Update Read Address *///////
//buffer reader
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if((VGA_PIXEL_X>`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1)) begin 
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
//				if (VGA_PIXEL_X==VGA_PIXEL_Y) begin
//					pixel_data_RGB332 = RED;
//				end
//				else begin
//					pixel_data_RGB332 = GREEN;
//				end
				VGA_READ_MEM_EN = 1'b1;
		end
end


//downsampler
reg cycle = 1'b0;
//reg [15:0] cameradata;


always @ (posedge PCLK) begin 
	if (VSYNC) begin 
		X_ADDR = 0;
		cycle = 0;
		W_EN = 0;
		pixel_data_RGB332[7:0] = 0;
		//cameradata[15:0] = 0;
	end
	else begin 
		if (!HREF) begin
			X_ADDR = 0;
			cycle = 0;
			W_EN = 0;
			pixel_data_RGB332[7:0] = 0;
			//cameradata[15:0] = 0;
		end
		else begin
			if (!cycle ) begin
				//cameradata[15:0] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13], GPIO_1_D[12], GPIO_1_D[11], GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				cycle = 1'b1;
				W_EN = 0;
				X_ADDR = X_ADDR;
				//pixel_data_RGB332[7:5] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13]};
				//pixel_data_RGB332[4:2] = {GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				pixel_data_RGB332[1:0] = {GPIO_1_D[12], GPIO_1_D[11]}; //something is wrong with the cycles, blue being output before red/green, but this code works
			end
			else begin
				//cameradata[15:8] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13], GPIO_1_D[12], GPIO_1_D[11], GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				//pixel_data_RGB332[7:0] = {cameradata[4], cameradata[3], cameradata[2], cameradata[10], cameradata[9], cameradata[8], cameradata[15], cameradata[14]};
				pixel_data_RGB332[7:5] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13]};
				pixel_data_RGB332[4:2] = {GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
				//pixel_data_RGB332[1:0] = {GPIO_1_D[12], GPIO_1_D[11]};
				cycle = 1'b0;
				W_EN = 1;
				X_ADDR = X_ADDR + 1'b1;
			end
		end
	end
end

	
endmodule 
