import serial
import time
import keyboard
import threading

class CelluveyorController:
    def __init__(self, port='COM3', baudrate=115200):
        self.arduino = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # Wait for Arduino to reset
        self.running = True
        self.current_speed = 1000
        self.monitoring = False
        
    def send_command(self, cmd):
        self.arduino.write(f"{cmd}\n".encode())
        time.sleep(0.1)  # Small delay to ensure command is processed
        
    def set_speed(self, speed):
        if 500 <= speed <= 2000:
            self.current_speed = speed
            self.send_command(f"S{speed}")
            print(f"Speed set to {speed} microseconds")
        else:
            print("Speed must be between 500 and 2000 microseconds")
            
    def move_steps(self, steps, direction='F'):
        if direction not in ['F', 'B']:
            print("Direction must be 'F' for forward or 'B' for backward")
            return
        self.send_command(f"{direction}{steps}")
        print(f"Moving {'forward' if direction == 'F' else 'backward'} {steps} steps")
        
    def emergency_stop(self):
        self.send_command("E")
        print("Emergency Stop Activated!")
        
    def get_position(self):
        self.send_command("P")
        response = self.arduino.readline().decode().strip()
        if response.startswith("POS:"):
            pos = response[4:].split(",")
            return [int(p) for p in pos]
        return None
        
    def monitor_position(self):
        self.monitoring = True
        while self.monitoring:
            pos = self.get_position()
            if pos:
                print(f"\rPosition - X: {pos[0]} Y: {pos[1]} Z: {pos[2]}", end="")
            time.sleep(0.5)
            
    def start_monitoring(self):
        self.monitor_thread = threading.Thread(target=self.monitor_position)
        self.monitor_thread.start()
        
    def stop_monitoring(self):
        self.monitoring = False
        if hasattr(self, 'monitor_thread'):
            self.monitor_thread.join()
            
    def close(self):
        self.stop_monitoring()
        self.arduino.close()

def main():
    try:
        # Create controller instance
        controller = CelluveyorController()
        print("Celluveyor Controller initialized")
        print("\nCommands:")
        print("F<steps> - Move forward")
        print("B<steps> - Move backward")
        print("S<speed> - Set speed (500-2000)")
        print("P - Toggle position monitoring")
        print("E - Emergency stop")
        print("Q - Quit")
        
        monitoring = False
        
        while True:
            cmd = input("\nEnter command: ").strip().upper()
            
            if cmd.startswith('F') or cmd.startswith('B'):
                try:
                    steps = int(cmd[1:])
                    controller.move_steps(steps, cmd[0])
                except ValueError:
                    print("Invalid step value")
                    
            elif cmd.startswith('S'):
                try:
                    speed = int(cmd[1:])
                    controller.set_speed(speed)
                except ValueError:
                    print("Invalid speed value")
                    
            elif cmd == 'P':
                if not monitoring:
                    controller.start_monitoring()
                    monitoring = True
                else:
                    controller.stop_monitoring()
                    monitoring = False
                    print("\n")  # New line after monitoring
                    
            elif cmd == 'E':
                controller.emergency_stop()
                
            elif cmd == 'Q':
                break
                
            else:
                print("Invalid command")
                
    except KeyboardInterrupt:
        print("\nProgram interrupted by user")
        
    finally:
        controller.close()
        print("Program terminated")

if __name__ == "__main__":
    main() 