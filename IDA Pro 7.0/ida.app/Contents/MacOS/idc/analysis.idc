//
//      Sample IDC program to automate IDA.
//
//      IDA can be run from the command line in the
//      batch (non-interactive) mode.
//
//      If IDA is started with
//
//              idag -A -Sanalysis.idc file
//
//      then this IDC file will be executed. It performs the following:
//
//        - the code segment is analyzed
//        - the output file is created
//        - IDA exits to OS
//
//      Feel free to modify this file as you wish
//      (or write your own script/plugin to automate IDA)
//
//      Since the script calls the qexit() function at the end,
//      it can be used in the batch files (use text mode idaw.exe)
//
//      NB: "idag -B file" is equivalent to the command line above
//

#include <idc.idc>

static main()
{
  // turn on coagulation of data in the final pass of analysis
  set_inf_attr(INF_AF, get_inf_attr(INF_AF) | AF_DODATA);

  msg("Waiting for the end of the auto analysis...\n");
  auto_wait();
  msg("\n\n------ Creating the output file.... --------\n");
  auto file = get_idb_path()[0:-4] + ".asm";
  WriteTxt(file, 0, BADADDR);           // create the assembler file
  msg("All done, exiting...\n");
  qexit(0);                              // exit to OS, error code 0 - success
}
