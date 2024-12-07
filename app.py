import yfinance as yf
from flask import Flask, jsonify, request

app = Flask(__name__)

def fetch_stock_data(symbol, interval="1m"):
    try:
        stock = yf.Ticker(symbol)
        history = stock.history(period="1d", interval=interval)
        if history.empty:
            return {"error": "No data available for the symbol"}
        
        latest_candle = history.iloc[-1]
        return {
            "symbol": symbol,
            "open": latest_candle['Open'],
            "high": latest_candle['High'],
            "low": latest_candle['Low'],
            "close": latest_candle['Close'],
            "volume": latest_candle['Volume'],
            "timestamp": latest_candle.name.strftime('%Y-%m-%d %H:%M:%S')
        }
    except Exception as e:
        return {"error": f"Failed to fetch stock data: {str(e)}"}

def evaluate_trading_logic(data, previous_data=None):
    if "error" in data:
        return data
    
    open_price = data["open"]
    high_price = data["high"]
    low_price = data["low"]
    close_price = data["close"]
    previous_close = previous_data["close"] if previous_data else None
    previous_open = previous_data["open"] if previous_data else None
    previous_high = previous_data["high"] if previous_data else None
    previous_low = previous_data["low"] if previous_data else None

    signal = "HOLD"
    pattern = "Unknown"

    if previous_close and close_price < open_price:
        if previous_close > previous_open and low_price < previous_low and high_price < previous_high:
            signal = "SELL"
            pattern = "Falling Window"
        elif close_price < open_price and high_price > open_price * 2 and close_price - low_price < 1:
            signal = "SELL"
            pattern = "Shooting Star"

    if previous_close and close_price > open_price:
        if previous_close < previous_open and low_price > previous_high:
            signal = "BUY"
            pattern = "Rising Window"
        elif close_price > open_price and previous_close < previous_open and open_price < previous_high:
            signal = "BUY"
            pattern = "Engulfing"

    return {
        "symbol": data["symbol"],
        "open": open_price,
        "high": high_price,
        "low": low_price,
        "close": close_price,
        "volume": data["volume"],
        "timestamp": data["timestamp"],
        "signal": signal,
        "pattern": pattern
    }

@app.route('/api/trade', methods=['GET'])
def trade_signal():
    symbol = request.args.get('symbol', 'AMZN').upper()
    stock_data = fetch_stock_data(symbol)

    previous_data = {
        "open": 218.0,
        "high": 220.0,
        "low": 216.5,
        "close": 219.0
    }

    evaluated_data = evaluate_trading_logic(stock_data, previous_data)
    
    # Print output to the console for debugging or monitoring
    print(f"Symbol: {evaluated_data.get('symbol', 'Unknown')}")
    print(f"Price: {evaluated_data.get('close', 'Unknown')}")
    print(f"Signal: {evaluated_data.get('signal', 'Unknown')}")
    print(f"Pattern: {evaluated_data.get('pattern', 'Unknown')}")
    
    return jsonify(evaluated_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
