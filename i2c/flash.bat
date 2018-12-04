@REM ask user if he is really sure
@echo flash to attiny2313a mcu?
@pause

@REM rebuild
@REM echo building..
@REM build.bat

@REM flash
@echo flashing..
avrdude -c usbasp -p t2313 -U flash:w:i2ctest.hex