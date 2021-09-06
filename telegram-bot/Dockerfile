FROM python:3.8

WORKDIR /telegram-bot

ENV PYTHONUNBUFFERED=1

COPY requirements.txt /tmp/
RUN pip install --no-cache-dir -r /tmp/requirements.txt

COPY src .

EXPOSE 8080

CMD ["python", "server.py"]
