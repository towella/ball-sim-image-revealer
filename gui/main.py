import tkinter as tk
from tkinter import filedialog as fd
import os

# Create main window
root = tk.Tk()
root.title("Image Revealer")
root.geometry("500x450")

current_row = 0
spacer = 20
left_pad = 10

# create elements

def get_tk_label(root, text, font_size=16):
    return tk.Label(root, text=text, font=("Arial", font_size))

def tk_button(root, text, command, column=0, y_padding=0):
    global current_row
    button = tk.Button(root, text=text, command=command)
    button.grid(row=current_row, column=column, pady=(y_padding, 0))
    current_row += 1

def tk_spinbox(root, label_text, default_text, from_num, to_num, y_padding=0, command=None):
    global current_row, left_pad
    label = get_tk_label(root, label_text)
    default = tk.StringVar(root)
    default.set(default_text)
    
    spinbox = tk.Spinbox(root, from_=from_num, to=to_num, textvariable=default, command=command)
    # verify keyboard inputs are numerical
    vcmd = (root.register(validate_spinbox), '%P')
    spinbox.configure(validate="key", validatecommand=vcmd)
    # activate command on keyboard enter (in case of keyboard entry)
    spinbox.bind("<Return>", command)

    label.grid(row=current_row, column=0, pady=(y_padding, 0), padx=(left_pad, 0), sticky="e")
    spinbox.grid(row=current_row, column=1, pady=(y_padding, 0))
    current_row += 1
    return spinbox

def tk_entry(root, label_text, y_padding=0, default_text="", ):
    global current_row
    label = get_tk_label(root, label_text)
    entry_text = tk.StringVar()
    entry_text.set(default_text)
    entry = tk.Entry(root, textvariable=entry_text)
    label.grid(row=current_row, column=0, pady=(y_padding, 0), padx=(left_pad, 0), stick="e")
    entry.grid(row=current_row, column=1, pady=(y_padding, 0))
    current_row += 1
    return entry

def file_selection(root, label_text, file_selector_command, button_text="select image", y_padding=0):
    global current_row
    label = get_tk_label(root, label_text)
    path = tk.StringVar()
    path.set("")
    entry = tk.Entry(root, textvariable=path)
    button = tk.Button(root, text=button_text, command= lambda: file_selector_command(path))
    label.grid(row=current_row, column=0, pady=(y_padding, 0), padx=(left_pad, 0), sticky="e")
    entry.grid(row=current_row, column=1, pady=(y_padding, 0))
    button.grid(row=current_row, column=2, pady=(y_padding, 0))
    current_row += 1
    return entry


# callbacks

def validate_spinbox(new_value):
    return new_value.isdigit() or new_value == ""

def select_image(entry_text):
    entry_text.set(fd.askopenfilename(
        title="Select an image",
        filetypes = (("PNG image", "*.png"), ("JPEG image", ["*.jpg", "*.jpeg", "*.JPEG"]))
    ))

def select_folder(entry_text):
    entry_text.set(fd.askdirectory(
        title="Select a folder",
        initialdir="~"
    ))

def update_total_duration(event=None):
    global total_duration_text, frame_dur_box, fps_box
    frames = frame_dur_box.get()
    fps = fps_box.get()
    if frames.isnumeric() and fps.isnumeric() and int(frames) > 0 and int(fps) > 0:
        total_duration_text["text"] = f"~{int(frame_dur_box.get()) // int(fps_box.get())} seconds"

def generate():
    global width_box, height_box, num_balls_box, ball_interval_box, frame_dur_box, reveal_path_entry, output_path_entry, video_name_entry, validation_label
    # validation
    if width_box.get() != "" and height_box.get() != "" and num_balls_box.get() != "" and ball_interval_box.get() != "" and \
        frame_dur_box.get() != "" and reveal_path_entry.get() != "" and output_path_entry.get() != "" and video_name_entry.get() != "":
        os.system(f"../ballSim {width_box.get()} {height_box.get()} {num_balls_box.get()} {ball_interval_box.get()} {frame_dur_box.get()} {reveal_path_entry.get()} | ../include/ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 720x1280 -i - -c:v h264 -pix_fmt yuv420p {output_path_entry.get()}/{video_name_entry.get()}.mov")
    else:
        validation_label["text"] = "Ensure all fields are filled and valid"


# gui

dimensions_label = get_tk_label(root, "Dimensions must fit in display to render", 11)
dimensions_label.grid(row=current_row, column=1, pady=(spacer, 0))
current_row += 1
width_box = tk_spinbox(root, "Width: ", "720", 1, 4000)
height_box = tk_spinbox(root, "Height: ", "1280", 1, 4000)

frame_dur_box = tk_spinbox(root, "Frame Duration: ", "1600", 1, 99999, spacer, command=update_total_duration)
fps_box = tk_spinbox(root, "FPS: ", "60", 1, 300, command=update_total_duration)

total_duration_label = get_tk_label(root, "Total Vid Duration: ")
total_duration_text = get_tk_label(root, "")
update_total_duration()
total_duration_label.grid(row=current_row, column=0, padx=(left_pad, 0), sticky="e")
total_duration_text.grid(row=current_row, column=1)
current_row += 1

num_balls_box = tk_spinbox(root, "Number of Balls: ", "695", 1, 99999, spacer)
ball_interval_box = tk_spinbox(root, "Ball Release Interval: ", "2", 1, 999)

reveal_path_entry = file_selection(root, "Reveal Image: ", select_image, "select image", spacer)
output_path_entry = file_selection(root, "Output Location: ", select_folder, "select folder")

video_name_entry = tk_entry(root, "Output video name: ", default_text="MyVideo")
label = get_tk_label(root, ".mov", 14)
label.grid(row=(current_row-1), column=2, sticky="w")

validation_label = get_tk_label(root, "", 11)
validation_label.grid(row=current_row, column=1, pady=(spacer, 0))
current_row += 1

tk_button(root, "Generate", generate, 1)


# Start the GUI event loop
root.mainloop()