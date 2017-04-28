# Servo Control
import time
import wiringpi

# use 'GPIO naming'
wiringpi.wiringPiSetupGpio()

# set pin 18 to be a PWM output
wiringpi.pinMode(18, wiringpi.GPIO.PWM_OUTPUT)
wiringpi.pinMode(17, wiringpi.GPIO.INPUT)

# set the PWM mode to milliseconds type
wiringpi.pwmSetMode(wiringpi.GPIO.PWM_MODE_MS)

# set up clock division
wiringpi.pwmSetClock(192)
wiringpi.pwmSetRange(2000)

delay_period = 0.01

wiringpi.pwmWrite(18,120)
time.sleep(1)

while True:
	for pulse in range(100, 200, 1):
		wiringpi.pwmWrite(18,pulse)
		time.sleep(delay_period)
	for pulse in range(200, 100, -1):
		wiringpi.pwmWrite(18,pulse)
		time.sleep(delay_period)

