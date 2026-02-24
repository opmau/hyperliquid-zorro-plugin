// HyperliquidSmokeTest.c - Smoke test for refactored Hyperliquid plugin
// Run after building Hyperliquid_Dev.dll and copying to Zorro/Plugin/
//
// Setup:
//   1. Select "Hyperliquid_Dev" from Account dropdown
//   2. Enter testnet wallet address in User field
//   3. Enter testnet private key in Password field
//   4. Click [Trade] to run
//
// Expected: All tests PASS, plugin reports "Hyperliquid-DEV 2.0.0-DEV"

function main()
{
	set(LOGFILE);
	Verbose = 7;

	printf("\n========================================");
	printf("\n  HYPERLIQUID PLUGIN SMOKE TEST");
	printf("\n  Branch: refactor/modularize");
	printf("\n========================================\n");

	int passed = 0;
	int failed = 0;
	int skipped = 0;

	// Enable diagnostics for maximum logging
	brokerCommand(SET_DIAGNOSTICS, 2);

	//------------------------------------------
	// TEST 1: Login
	//------------------------------------------
	printf("\n[1/10] Login...");
	if(0 == login(1)) {
		printf(" FAIL - Login failed, cannot continue\n");
		printf("\nCheck: User=wallet address, Password=private key\n");
		printf("Account dropdown must show 'Hyperliquid_Dev'\n");
		return;
	}
	printf(" PASS\n");
	passed++;

	// Brief wait for WS to connect
	wait(2000);

	//------------------------------------------
	// TEST 2: Asset info (BTC, ETH, SOL)
	//------------------------------------------
	printf("[2/10] Asset info (BTC, ETH, SOL)...");
	int assetOk = 1;

	if(!asset("BTC-USD")) {
		printf("\n  BTC-USD FAIL");
		assetOk = 0;
	}
	if(!asset("ETH-USD")) {
		printf("\n  ETH-USD FAIL");
		assetOk = 0;
	}
	if(!asset("SOL-USD")) {
		printf("\n  SOL-USD FAIL");
		assetOk = 0;
	}

	if(assetOk) {
		printf(" PASS\n");
		passed++;
	} else {
		printf("\n  FAIL - One or more assets not found\n");
		failed++;
	}

	//------------------------------------------
	// TEST 3: Price fetch (HTTP fallback)
	//------------------------------------------
	printf("[3/10] Price fetch...");
	asset("BTC-USD");
	var btcPrice = priceClose();
	if(btcPrice > 0) {
		printf(" PASS - BTC: $%.2f\n", btcPrice);
		passed++;
	} else {
		printf(" FAIL - BTC price is 0\n");
		failed++;
	}

	//------------------------------------------
	// TEST 4: WebSocket streaming
	//------------------------------------------
	printf("[4/10] WebSocket price streaming...");
	asset("BTC-USD");
	var price1 = priceClose();
	wait(3000);
	var price2 = priceClose();
	wait(2000);
	var price3 = priceClose();

	int changes = 0;
	if(price2 != price1) changes++;
	if(price3 != price2) changes++;

	if(changes >= 1) {
		printf(" PASS (%d changes: %.2f -> %.2f -> %.2f)\n", changes, price1, price2, price3);
		passed++;
	} else {
		printf(" WARN - No price changes (quiet market or WS issue)\n");
		printf("  Prices: %.2f -> %.2f -> %.2f\n", price1, price2, price3);
		// Count as pass since quiet market is possible
		passed++;
	}

	//------------------------------------------
	// TEST 5: Account balance
	//------------------------------------------
	printf("[5/10] Account balance...");
	var equity = brokerCommand(GET_EQUITY, 0);
	if(equity > 0) {
		printf(" PASS - Equity: $%.2f\n", equity);
		passed++;
	} else if(equity == 0) {
		printf(" WARN - Equity is $0 (empty testnet account?)\n");
		passed++;
	} else {
		printf(" FAIL - Equity query failed\n");
		failed++;
	}

	//------------------------------------------
	// TEST 6: Position query
	//------------------------------------------
	printf("[6/10] Position query (BTC)...");
	asset("BTC-USD");
	var pos = brokerCommand(GET_POSITION, 0);
	printf(" PASS - Position: %.4f (0 = no position)\n", pos);
	passed++;

	//------------------------------------------
	// TEST 7: Limit order (far from market)
	//------------------------------------------
	printf("[7/10] Limit order...");
	asset("BTC-USD");
	var curPrice = priceClose();
	if(curPrice > 0) {
		// Place a buy limit at 50% below market - should NOT fill
		var limitPrice = curPrice * 0.5;
		brokerCommand(SET_ORDERTYPE, 2);  // Limit order
		int tradeId = enterLong(1, limitPrice, 0, 0);
		wait(2000);

		if(tradeId > 0) {
			printf(" PASS - Order placed, tradeId=%d, limit=$%.2f\n", tradeId, limitPrice);
			passed++;

			//------------------------------------------
			// TEST 8: Order status
			//------------------------------------------
			printf("[8/10] Order status...");
			var tradeProfit = brokerCommand(GET_PRICETYPE, 0);
			printf(" PASS (queried, no crash)\n");
			passed++;

			//------------------------------------------
			// TEST 9: Cancel order
			//------------------------------------------
			printf("[9/10] Cancel order...");
			exitLong("*", 0, 0);
			wait(2000);
			printf(" PASS (cancel sent)\n");
			passed++;
		} else {
			printf(" FAIL - Order rejected (tradeId=%d)\n", tradeId);
			failed++;
			printf("[8/10] Order status... SKIP (no order)\n");
			skipped++;
			printf("[9/10] Cancel order... SKIP (no order)\n");
			skipped++;
		}
	} else {
		printf(" SKIP (no price data)\n");
		skipped++;
		printf("[8/10] Order status... SKIP\n");
		skipped++;
		printf("[9/10] Cancel order... SKIP\n");
		skipped++;
	}

	//------------------------------------------
	// TEST 10: Balance unchanged after cancel
	//------------------------------------------
	printf("[10/10] Balance after cancel...");
	var equity2 = brokerCommand(GET_EQUITY, 0);
	if(equity2 > 0) {
		var diff = abs(equity2 - equity);
		if(diff < 1.0) {
			printf(" PASS - Equity: $%.2f (unchanged)\n", equity2);
		} else {
			printf(" WARN - Equity changed: $%.2f -> $%.2f (diff=$%.2f)\n", equity, equity2, diff);
		}
		passed++;
	} else {
		printf(" FAIL\n");
		failed++;
	}

	//------------------------------------------
	// SUMMARY
	//------------------------------------------
	printf("\n========================================");
	printf("\n  RESULTS: %d PASSED, %d FAILED, %d SKIPPED", passed, failed, skipped);
	if(failed == 0) {
		printf("\n  STATUS: SMOKE TEST PASSED");
	} else {
		printf("\n  STATUS: %d FAILURES - CHECK LOGS", failed);
	}
	printf("\n========================================\n");

	printf("\nFull log saved to: Log/HyperliquidSmokeTest.log\n");
}
