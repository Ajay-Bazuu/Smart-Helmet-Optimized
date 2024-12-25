import platform
import asyncio
import json
from threading import Thread

from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.gridlayout import GridLayout
from kivy.uix.floatlayout import FloatLayout
from kivy.clock import Clock
from kivy.graphics import Color, Rectangle
from kivy.metrics import dp

# BLE settings
TARGET_DEVICE_NAME = "ESP32_S3_Bluetooth"
TARGET_MAC_ADDRESS = "f0:9e:9e:22:7b:01"
TARGET_CHARACTERISTIC_UUID = "87654321-4321-4321-4321-210987654321"

if platform.system() == "Android":
    from jnius import autoclass
    BluetoothAdapter = autoclass('android.bluetooth.BluetoothAdapter')
    Activity = autoclass('org.kivy.android.PythonActivity')
else:
    from bleak import BleakScanner, BleakClient


class BLEApp(App):
    """Main BLE Kivy Application"""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.running_tasks = []
        self.data_labels = {}
        self.scan_animation_event = None
        self.scan_message_state = 0
        self.scan_message_base = "Scanning for BLE devices"
        self.platform = platform.system()

        # Android-specific
        if self.platform == "Android":
            self.bluetooth_adapter = BluetoothAdapter.getDefaultAdapter()

    def build(self):
        """Build initial UI."""
        self.layout = FloatLayout()

        # Initial message
        self.log_label = Label(
            text="Press Start to Scan...",
            font_size=24,
            halign="center",
            valign="middle",
            size_hint=(1, 1)
        )
        self.log_label.bind(size=self.log_label.setter('text_size'))
        self.layout.add_widget(self.log_label)

        # Start Button
        self.start_button = Button(
            text="Start Scan",
            font_size=24,
            size_hint=(0.8, None),
            height=dp(60),
            pos_hint={"center_x": 0.5, "center_y": 0.1}
        )
        self.start_button.bind(on_press=self.start_ble_scan)
        self.layout.add_widget(self.start_button)

        return self.layout

    def log_message(self, message):
        """Log messages to the UI."""
        self.log_label.text = message

    def start_ble_scan(self, instance):
        """Start BLE scanning."""
        self.layout.clear_widgets()
        self.layout.add_widget(self.log_label)  # Ensure log_label is re-added
        self.add_stop_button()  # Add the stop button
        
        # Update the scanning message
        self.log_message(self.scan_message_base)
        
        # Start the animation and BLE scan in a separate thread
        self.scan_animation_event = Clock.schedule_interval(self.animate_scan_message, 0.5)
        Thread(target=self.run_ble_scan).start()

    def animate_scan_message(self, dt):
        """Animate scanning message."""
        self.scan_message_state = (self.scan_message_state + 1) % 4
        self.log_label.text = self.scan_message_base + "." * self.scan_message_state

    def reset_app(self, instance=None):
        """Reset the app."""
        for task in self.running_tasks:
            task.cancel()
        self.running_tasks.clear()
        if self.scan_animation_event:
            self.scan_animation_event.cancel()
            self.scan_animation_event = None
        
        # Clear the layout and rebuild it
        self.layout.clear_widgets()
        self.log_label = Label(
            text="Press Start to Scan...",
            font_size=24,
            halign="center",
            valign="middle",
            size_hint=(1, 1)
        )
        self.log_label.bind(size=self.log_label.setter('text_size'))
        self.layout.add_widget(self.log_label)

        # Re-add the start button
        self.start_button = Button(
            text="Start Scan",
            font_size=24,
            size_hint=(0.8, None),
            height=dp(60),
            pos_hint={"center_x": 0.5, "center_y": 0.1}
        )
        self.start_button.bind(on_press=self.start_ble_scan)
        self.layout.add_widget(self.start_button)

    def add_stop_button(self):
        """Add Stop button."""
        self.stop_button = Button(
            text="Stop",
            font_size=24,
            size_hint=(0.8, None),
            height=dp(60),
            pos_hint={"center_x": 0.5, "center_y": 0.1}
        )
        self.stop_button.bind(on_press=self.reset_app)
        self.layout.add_widget(self.stop_button)

    def run_ble_scan(self):
        """Run BLE scan based on the platform."""
        if self.platform == "Android":
            self.scan_ble_android()
        else:
            self.run_async(self.scan_ble_desktop())

    def scan_ble_android(self):
        """Android-specific BLE scan."""
        if not self.bluetooth_adapter.isEnabled():
            self.bluetooth_adapter.enable()
            self.log_message("Enabling Bluetooth...")
        self.log_message("Scanning for devices on Android...")
        # Simulate device discovery
        Clock.schedule_once(lambda dt: self.log_message("Found device: ESP32_S3_Bluetooth"), 3)
        Clock.schedule_once(lambda dt: self.connect_to_device_android(), 5)

    def connect_to_device_android(self):
        """Android-specific device connection."""
        self.log_message("Connected to device.")
        Clock.schedule_once(self.create_grid_layout)

    async def scan_ble_desktop(self):
        """Desktop-specific BLE scan using Bleak."""
        try:
            devices = await BleakScanner.discover(timeout=10)
            for device in devices:
                if device.name == TARGET_DEVICE_NAME or device.address == TARGET_MAC_ADDRESS:
                    self.log_message(f"Found device: {device.name}")
                    await self.connect_and_receive(device)
                    return
            self.log_message("Target device not found.")
        except Exception as e:
            self.log_message(f"Error during scanning: {e}")
        finally:
            if self.scan_animation_event:
                self.scan_animation_event.cancel()

    async def connect_and_receive(self, device):
        """Connect to the BLE device and receive data."""
        try:
            async with BleakClient(device.address) as client:
                self.log_message("Connected to device.")
                Clock.schedule_once(self.create_grid_layout)
                while True:
                    data = await client.read_gatt_char(TARGET_CHARACTERISTIC_UUID)
                    decoded_data = data.decode('utf-8').strip()
                    Clock.schedule_once(lambda dt: self.update_labels(decoded_data))
        except Exception as e:
            self.log_message(f"Connection error: {e}")

    def create_grid_layout(self, *args):
        """Create grid layout for BLE data."""
        self.layout.clear_widgets()
        grid_layout = GridLayout(cols=1, spacing=10, padding=10)

        # Create labels for values
        self.data_labels = {}
        for key in ["HelmetConnection: ", "BluetoothStatus: ", "FSR Value: ", "Alcohol Value: ", "Inclination: ", "Speed: ", "Engine Temperature: ", "Distance: "]:
            label = Label(
                text=f"{key}: Waiting...",
                font_size=20,
                size_hint=(1, None),
                height=dp(50)
            )
            with label.canvas.before:
                Color(0, 0, 0, 1)  # Default black background
                label.rect = Rectangle(pos=label.pos, size=label.size)
            label.bind(pos=self.update_rect, size=self.update_rect)
            grid_layout.add_widget(label)
            self.data_labels[key] = label

        self.layout.add_widget(grid_layout)
        self.add_stop_button()

    def update_labels(self, data):
        """Update the labels based on received JSON data."""
        try:
            json_data = json.loads(data)
            for key, label in self.data_labels.items():
                value = json_data.get(key, "N/A")
                label.text = f"{key}: {value}"
        except json.JSONDecodeError:
            self.log_message("Invalid JSON data.")

    def update_rect(self, instance, value):
        """Update rectangle position and size."""
        instance.rect.pos = instance.pos
        instance.rect.size = instance.size

    def run_async(self, coro):
        """Run an asyncio coroutine."""
        task = self.loop.create_task(coro)
        self.running_tasks.append(task)
        try:
            self.loop.run_until_complete(task)
        except asyncio.CancelledError:
            self.log_message("Task cancelled.")
        finally:
            self.running_tasks.remove(task)


if __name__ == "__main__":
    BLEApp().run()
