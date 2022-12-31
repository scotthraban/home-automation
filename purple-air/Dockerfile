FROM python:3.11

WORKDIR /purple-air

ENV PYTHONUNBUFFERED=1

COPY requirements.txt /tmp/
RUN pip install --no-cache-dir -r /tmp/requirements.txt

COPY src .

CMD ["python", "purple-air-to-mqtt.py"]
