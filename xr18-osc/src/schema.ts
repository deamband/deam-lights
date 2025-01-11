export interface XInfo {
  ip: string;
  name: string;
  type: string;
  version: string;
}

export interface IncomingMessage {
  address: string;
  args: any[];
}

export type MessageArg = { type: string; value: string };

export interface MeterLevels {
  //Output structure
  1: number; //channel 1 - prefade
  2: number; //channel 2 - prefade
  3: number; //channel 3 - prefade
  4: number; //channel 4 - prefade
  5: number; //channel 5 - prefade
  6: number; //channel 6 - prefade
  7: number; //channel 7 - prefade
  8: number; //channel 8 - prefade
  9: number; //channel 9 - prefade
  10: number; //channel 10 - prefade
  11: number; //channel 11 - prefade
  12: number; //channel 12 - prefade
  13: number; //channel 13 - prefade
  14: number; //channel 14 - prefade
  15: number; //channel 15 - prefade
  16: number; //channel 16 - prefade
  auxL: number; //aux in channel - prefade (left)
  auxR: number; //aux in channel - prefade (right)
  fx1PreL: number; //Effect prefade
  fx1PreR: number;
  fx2PreL: number;
  fx2PreR: number;
  fx3PreL: number;
  fx3PreR: number;
  fx4PreL: number;
  fx4PreR: number;
  bus1Pre: number; //Bus prefade
  bus2Pre: number;
  bus3Pre: number;
  bus4Pre: number;
  bus5Pre: number;
  bus6Pre: number;
  fx1SendPre: number; //Effect send prefade
  fx2SendPre: number;
  fx3SendPre: number;
  fx4SendPre: number;
  mainPostL: number; //Main mix out postfade (left)
  mainPostR: number; //Main mix out postfade (right)
  monL: number; //Monitor out (left)
  monR: number; //Monitor out (right)
}
