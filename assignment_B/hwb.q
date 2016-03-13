transac_record:("PSFI";",") 0:`data.csv

transac_table:flip `time`ticker`price`size!transac_record

transac_table: update date: `date$transac_table`time from transac_table

vwap:select vwap:(sum price*size)%(sum size) by date,ticker from transac_table

save `:vwap.csv
