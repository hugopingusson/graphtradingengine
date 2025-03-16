#include <iostream>

#include "DataReader/class_DataReader.h"
#include "Helper/class_FutureHelper.h"
#include "Logger/class_Logger.h"
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    // DataReader data_reader = DataReader();
    // auto test = data_reader.get_cme_market_data_table("EURUSD","2024-04-29");

    Logger logger = Logger();
    logger.log_error("Ceci est une error");

    return 1;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.