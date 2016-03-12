transac_record:("PSFID";",") 0:`data.csv

transac_table:flip `time`ticker`price`size`date!transac_record

vwap:select vwap:(sum price*size)%(sum size) by date,ticker from transac_table

save `:vwap.csv
