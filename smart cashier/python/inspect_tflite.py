# inspect_tflite.py
import numpy as np

def get_interpreter(model_path: str):
    # Try tflite-runtime first (lightweight), fallback to tensorflow
    try:
        from tflite_runtime.interpreter import Interpreter
        return Interpreter(model_path=model_path), "tflite_runtime"
    except Exception:
        from tensorflow.lite.python.interpreter import Interpreter
        return Interpreter(model_path=model_path), "tensorflow"

MODEL_PATH = "model_unquant.tflite"

interpreter, backend = get_interpreter(MODEL_PATH)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("Backend:", backend)
print("\n== Inputs ==")
for i, d in enumerate(input_details):
    print(f"[{i}] name={d['name']}, shape={d['shape']}, dtype={d['dtype']}, quant={d.get('quantization_parameters', d.get('quantization'))}")

print("\n== Outputs ==")
for i, d in enumerate(output_details):
    print(f"[{i}] name={d['name']}, shape={d['shape']}, dtype={d['dtype']}, quant={d.get('quantization_parameters', d.get('quantization'))}")
