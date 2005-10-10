"""A readable 150 lines script against 1312 lines of that...

PLearn serialization of my experiment::

    *184 -> PipelineSequentialValidation(
        learner = *81 -> ApTrader(
            advisor = *3 -> EmbeddedPLearnerAdvisor(
                dataset_wrapper = *1 -> DisregardRowsVMatrix( maximum_length = 120 ),
                default_class = "asset",
                horizon = 0,
                input_sefams = [ "":"pseudo_price_return1" ],
                learner = *2 -> LinearRegressor(
                    include_bias = 0,
                    output_learned_weights = 1,
                    weight_decay = 1e-05
                    ),
                target_sefams = [ "target_returns":"any" ],
                train_on_test_input = 1
                ),
            asset_manager = *29 -> AssetManager(
                series_names = [
                    "Russell1000":"asset",
                    "Russell2000":"asset",
                    "cyymmdd_date":"none",
                    "ldom":"none",
                    "risk_free_rate":"risk_free_rate",
                    "S&P500":"benchmark",
                    "lse-us-index":"benchmark",
                    "short-lse-us-index":"benchmark",
                    "target_returns":"global_preproc"
                    ],
                class_families = {
                    "risk_free_rate" : [
                        "return":*4 -> SeriesFamily(
                                members = [ "us-tbills:yield:return" ],
                                missing_fillin = 1
                                )
                        ],
                    "none" : [ "any":*5 -> SeriesFamily( ) ],
                    "benchmark" : [ "price":*6 -> SeriesFamily( members = [ "us-spx-daily:close:level" ] ) ],
                    "asset" : [
                        "price":*7 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "close:level"
                                ),
                        "open_price":*8 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "open:level"
                                ),
                        "high_price":*9 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "high:level"
                                ),
                        "low_price":*10 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "low:level"
                                ),
                        "close_price":*11 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "close:level"
                                ),
                        "pseudo_price":*12 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "pseudo:close:level"
                                ),
                        "previous_close":*13 -> SeriesFamily(
                                missing_fillin = 0,
                                tag = "last:close:level"
                                ),
                        "lastcon_open":*14 -> SeriesFamily(
                                missing_fillin = 0,
                                tag = "last_contract:open:level"
                                ),
                        "contract_size":*15 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "contract-size"
                                ),
                        "rollover":*16 -> SeriesFamily(
                                missing_fillby = 0,
                                tag = "rollover"
                                ),
                        "volume":*17 -> SeriesFamily(
                                missing_fillby = 0,
                                tag = "volume:level"
                                ),
                        "open_interest":*18 -> SeriesFamily(
                                missing_fillby = 0,
                                tag = "open-interest:level"
                                ),
                        "asset_currency":*19 -> SeriesFamily( ),
                        "pseudo_price_return1":*20 -> SeriesFamily(
                                missing_fillin = 1,
                                tag = "pseudo:close:level:ts_return1"
                                )
                        ],
                    "global_preproc" : [ "any":*21 -> SeriesFamily( ) ]
                    },
                series_families = {
                    "target_returns" : [
                        "any":*22 -> SeriesFamily(
                                members = [ "short-lse-us-index:ts_return1" ],
                                missing_fillin = 1
                                )
                        ],
                    "lse-us-index" : [ "price":*23 -> SeriesFamily( members = [ "lse-us-index" ] ) ],
                    "ldom" : [ "any":*24 -> SeriesFamily( members = [ "last_day_of_month" ] ) ],
                    "Russell1000" : [
                        "asset_currency":*25 -> SeriesFamily(
                                members = [ "USDollar:cash:close:level" ],
                                missing_fillin = 1
                                )
                        ],
                    "Russell2000" : [
                        "asset_currency":*26 -> SeriesFamily(
                                members = [ "USDollar:cash:close:level" ],
                                missing_fillin = 1
                                )
                        ],
                    "cyymmdd_date" : [ "any":*27 -> SeriesFamily( members = [ "Date" ] ) ],
                    "short-lse-us-index" : [ "price":*28 -> SeriesFamily( members = [ "short-lse-us-index" ] ) ]
                    },
                pseudoprice_family = "pseudo_price",
                previousclose_family = "previous_close",
                lastconopen_family = "lastcon_open",
                tradable_family = "",
                last_day_of_month_series = "ldom",
                cyymmdd_series = "cyymmdd_date",
                num_obs_year = 252
                ),
            decision_plugins = [
                *42 -> ExogenousExposureNOCDecision(
                    global_leverage = -1,
                    asset_leverage = [ ],
                    consider_absolute_positions = 1,
                    exogenous_trader = *41 -> ApTrader(
                        advisor = *30 -> PythonCodeAdvisor(
                            code = "advisor_output = [ 1 ]",
                            init_code = ""
                            ),
                        asset_manager = *37 -> AssetManager(
                            series_names = [
                                "lse-us-index":"asset",
                                "cyymmdd_date":"none",
                                "ldom":"none",
                                "risk_free_rate":"risk_free_rate"
                                ],
                            class_families = {
                                "risk_free_rate" : [ "return":*4; ],
                                "none" : [ "any":*5; ],
                                "asset" : [
                                    "price":*31 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            ),
                                    "open_price":*32 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            ),
                                    "high_price":*33 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            ),
                                    "low_price":*34 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            ),
                                    "close_price":*35 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            ),
                                    "pseudo_price":*36 -> SeriesFamily(
                                            members = [ "lse-us-index" ],
                                            missing_fillin = 1
                                            )
                                    ]
                                },
                            series_families = {
                                "cyymmdd_date" : [ "any":*27; ],
                                "ldom" : [ "any":*24; ]
                                },
                            pseudoprice_family = "pseudo_price",
                            previousclose_family = "",
                            lastconopen_family = "",
                            tradable_family = "",
                            last_day_of_month_series = "ldom",
                            cyymmdd_series = "cyymmdd_date",
                            num_obs_year = 252
                            ),
                        decision_plugins = [
                            *38 -> NumberOfContractsDecision(
                                global_leverage = 1,
                                asset_leverage = [ ],
                                consider_absolute_positions = 1,
                                name = "NumberOfContractsDecision"
                                )
                            ],
                        cost_plugins = [ ],
                        execution_plugins = [
                            *39 -> BasicExecution(
                                allow_execution_only_on_dates = [ ],
                                name = "BasicExecution"
                                ),
                            *40 -> MarkToMarketExecution( name = "MarkToMarketExecution" )
                            ],
                        test_performance_plugins = [ ],
                        initial_margin = 100000000.0,
                        save_advisor_trader_states = 1,
                        save_plugin_outputs = 0,
                        test_start_time = -1
                        ),
                    name = "ExogenousExposureNOCDecision"
                    ),
                *43 -> IntegralizeDecision(
                    truncate = 0,
                    nonnegative = 0,
                    name = "IntegralizeDecision"
                    ),
                *44 -> ThresholdDecision(
                    assetwise_absolute_threshold = 5,
                    name = "ThresholdDecision",
                    portfolio_relative_threshold = 0.02
                    )
                ],
            cost_plugins = [ ],
            execution_plugins = [
                *45 -> BasicExecution(
                    allow_execution_only_on_dates = [ ],
                    name = "BasicExecution"
                    ),
                *46 -> BasicTrCostExecution(
                    global_fixed_cost = 0,
                    global_unit_cost = 0,
                    global_proportional_cost = 0.001,
                    asset_fixed_cost = [ ],
                    asset_proportional_cost = [ ],
                    name = "BasicTrCostExecution"
                    ),
                *47 -> MarkToMarketExecution( name = "MarkToMarketExecution" )
                ],
            test_performance_plugins = [
                *48 -> PerfPortfolioMaxDrawdown(
                    name = "1mo",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    horizon = 1
                    ),
                *49 -> PerfPortfolioMaxDrawdown(
                    name = "3mo",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    horizon = 3
                    ),
                *50 -> PerfPortfolioReturn(
                    name = "Cash_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 1
                    ),
                *51 -> PerfPortfolioReturn(
                    name = "Cash_month_so_far",
                    month_so_far = 1,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 1
                    ),
                *52 -> PerfPortfolioReturn(
                    name = "Cash_monthly",
                    month_so_far = 0,
                    monthly = 1,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 1
                    ),
                *53 -> PerfPortfolioReturn(
                    name = "Cash_strict_monthly",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 1
                    ),
                *54 -> PerfPortfolioReturn(
                    name = "LSE_US_Index_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "lse-us-index",
                    bench_only = 1
                    ),
                *55 -> PerfPortfolioReturn(
                    name = "SP500_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 1
                    ),
                *56 -> PerfPortfolioReturn(
                    name = "SP500_month_so_far",
                    month_so_far = 1,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 1
                    ),
                *57 -> PerfPortfolioReturn(
                    name = "SP500_monthly",
                    month_so_far = 0,
                    monthly = 1,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 1
                    ),
                *58 -> PerfPortfolioReturn(
                    name = "SP500_strict_monthly",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 1
                    ),
                *59 -> PerfPortfolioReturn(
                    name = "daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "",
                    bench_only = 0
                    ),
                *60 -> PerfPortfolioTurnover(
                    name = "daily_portfolio_turnover",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0
                    ),
                *61 -> PerfPortfolioMaxDrawdown(
                    name = "historical",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    horizon = -1
                    ),
                *62 -> PerfPortfolioVaR(
                    name = "level95",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    var_level = 0.95,
                    var_kind = "riskmetrics",
                    output_type = "relative",
                    horizon = 10,
                    decay_factor = 0.94
                    ),
                *63 -> PerfPortfolioVaR(
                    name = "level99",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    var_level = 0.99,
                    var_kind = "riskmetrics",
                    output_type = "relative",
                    horizon = 10,
                    decay_factor = 0.94
                    ),
                *64 -> PerfPortfolioReturn(
                    name = "month_so_far",
                    month_so_far = 1,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "",
                    bench_only = 0
                    ),
                *65 -> PerfPortfolioReturn(
                    name = "monthly",
                    month_so_far = 0,
                    monthly = 1,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "",
                    bench_only = 0
                    ),
                *66 -> PerfPortfolioNetExposure(
                    name = "ne/daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0
                    ),
                *67 -> PerfAssetwiseReturn(
                    name = "port_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0
                    ),
                *68 -> PerfPortfolioEffectiveLeverage(
                    name = "portfolio_effective_leverage",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0
                    ),
                *69 -> PerfPortfolioReturn(
                    name = "strict_monthly",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    bench_riskfree = 0,
                    bench_index = "",
                    bench_only = 0
                    ),
                *70 -> PerfPortfolioTurnover(
                    name = "strict_monthly_portfolio_turnover",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1
                    ),
                *71 -> PerfPortfolioReturn(
                    name = "vsCash_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 0
                    ),
                *72 -> PerfPortfolioReturn(
                    name = "vsCash_month_so_far",
                    month_so_far = 1,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 0
                    ),
                *73 -> PerfPortfolioReturn(
                    name = "vsCash_monthly",
                    month_so_far = 0,
                    monthly = 1,
                    strict_monthly = 0,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 0
                    ),
                *74 -> PerfPortfolioReturn(
                    name = "vsCash_strict_monthly",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    bench_riskfree = 1,
                    bench_index = "",
                    bench_only = 0
                    ),
                *75 -> PerfPortfolioReturn(
                    name = "vsSP500_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 0
                    ),
                *76 -> PerfPortfolioReturn(
                    name = "vsSP500_month_so_far",
                    month_so_far = 1,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 0
                    ),
                *77 -> PerfPortfolioReturn(
                    name = "vsSP500_monthly",
                    month_so_far = 0,
                    monthly = 1,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 0
                    ),
                *78 -> PerfPortfolioReturn(
                    name = "vsSP500_strict_monthly",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 1,
                    bench_riskfree = 0,
                    bench_index = "S&P500",
                    bench_only = 0
                    ),
                *79 -> PerfPortfolioReturn(
                    name = "vs_LSE_US_Index_daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    bench_riskfree = 0,
                    bench_index = "lse-us-index",
                    bench_only = 0
                    ),
                *80 -> PerfPortfolioHitRatio(
                    name = "whr/daily",
                    month_so_far = 0,
                    monthly = 0,
                    strict_monthly = 0,
                    exposure_weighted = 1
                    )
                ],
            initial_margin = 100000000.0,
            save_advisor_trader_states = 1,
            save_plugin_outputs = 0,
            test_start_time = -1
            ),
        dataset = *82 -> *0;,
        expdir = "expdir_2005_06_21_21:56:52",
        init_train_size = 121,
        last_test_time = -1,
        statnames = [
            "E[test.E[daily_relative_return]]",
            "STDDEV[test.E[daily_relative_return]]",
            "SHARPERATIO[test.E[daily_relative_return]]",
            "MIN[test.E[daily_relative_return]]",
            "MAX[test.E[daily_relative_return]]",
            "SKEW[test.E[daily_relative_return]]",
            "KURT[test.E[daily_relative_return]]",
            "E[test.E[strict_monthly_relative_return]]",
            "STDDEV[test.E[strict_monthly_relative_return]]",
            "SHARPERATIO[test.E[strict_monthly_relative_return]]",
            "MIN[test.E[strict_monthly_relative_return]]",
            "MAX[test.E[strict_monthly_relative_return]]",
            "E[test.E[vsCash_strict_monthly_relative_return]]",
            "STDDEV[test.E[vsCash_strict_monthly_relative_return]]",
            "SHARPERATIO[test.E[vsCash_strict_monthly_relative_return]]",
            "E[test.E[vsSP500_strict_monthly_relative_return]]",
            "STDDEV[test.E[vsSP500_strict_monthly_relative_return]]",
            "SHARPERATIO[test.E[vsSP500_strict_monthly_relative_return]]",
            "E[test.E[level95_riskmetrics_relative_var]]",
            "STDDEV[test.E[level95_riskmetrics_relative_var]]",
            "SKEW[test.E[level95_riskmetrics_relative_var]]",
            "MAX[test.E[level95_riskmetrics_relative_var]]",
            "E[test.E[level99_riskmetrics_relative_var]]",
            "STDDEV[test.E[level99_riskmetrics_relative_var]]",
            "SKEW[test.E[level99_riskmetrics_relative_var]]",
            "MAX[test.E[level99_riskmetrics_relative_var]]",
            "LAST[test.E[historical_max_drawdown]]",
            "LAST[test.E[historical_drawdown_ratio]]",
            "LAST[test.E[historical_drawdown_duration]]",
            "LAST[test.E[historical_drawdown_recovery_duration]]",
            "E[test.E[whr/daily_weighted_hit_ratio]]",
            "E[test.E[ne/daily_net_exposure]]",
            "E[test.E[portfolio_effective_leverage]]",
            "STDDEV[test.E[portfolio_effective_leverage]]",
            "MIN[test.E[portfolio_effective_leverage]]",
            "MAX[test.E[portfolio_effective_leverage]]",
            "E[test.E[daily_portfolio_turnover]]",
            "STDDEV[test.E[daily_portfolio_turnover]]",
            "MIN[test.E[daily_portfolio_turnover]]",
            "MAX[test.E[daily_portfolio_turnover]]",
            "E[test.E[strict_monthly_portfolio_turnover]]",
            "STDDEV[test.E[strict_monthly_portfolio_turnover]]",
            "MIN[test.E[strict_monthly_portfolio_turnover]]",
            "MAX[test.E[strict_monthly_portfolio_turnover]]",
            "E[test.E[LSE_US_Index_daily_relative_return]]",
            "STDDEV[test.E[LSE_US_Index_daily_relative_return]]",
            "E[test.E[LSE_US_Index_daily_absolute_return]]",
            "STDDEV[test.E[LSE_US_Index_daily_absolute_return]]",
            "E[test.E[vs_LSE_US_Index_daily_absolute_return]]",
            "STDDEV[test.E[vs_LSE_US_Index_daily_absolute_return]]",
            "E[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "STDDEV[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "SHARPERATIO[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "MIN[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "MAX[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "SKEW[test.E[vs_LSE_US_Index_daily_relative_return]]",
            "KURT[test.E[vs_LSE_US_Index_daily_relative_return]]"
            ],
        provide_learner_expdir = 1,
        save_test_costs = 1,
        report_stats = 1,
        save_final_model = 0,
        save_initial_model = 0,
        save_initial_seqval = 0,
        save_test_outputs = 1,
        save_data_sets = 0,
        save_stat_collectors = 0,
        save_sequence_stats = 0,
        report_memory_usage = 0,
        pipeline = *183 -> PipeLineVMatrix(
            date_format = "fdate",
            pipeline = *182 -> PipeLine(
                verbose_propagate = 0,
                build_path = [
                    *84 -> VMatPipeStage(
                        stage_name = "fetched_data",
                        instages = [ ],
                        dominate_inputs_by = "",
                        data = *83 -> AutoVMatrix( specification = "/projects/finance/Desjardins/longshort_equity/data/ExtendedWorldData.vmat" ),
                        date_format = "fdate"
                        ),
                    *85 -> ResamplePipeStage(
                        stage_name = "first_resample",
                        instages = [ *84; ],
                        dominate_inputs_by = "id",
                        intersect_calendars = 1,
                        lower_fdate = 990101,
                        strip_all_missings = 0,
                        unite_calendars = 0,
                        upper_fdate = 1041231
                        ),
                    *90 -> SelectPipeStage(
                        stage_name = "select__first_resample",
                        instages = [ *85; ],
                        dominate_inputs_by = "",
                        args = [
                            *86 -> PipeStageArguments(
                                constraints = [ { "id" : "russel.*index" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "select__first_resample",
                                    "id" : "{id}"
                                    }
                                ),
                            *87 -> PipeStageArguments(
                                constraints = [ { "id" : "Russell.*open:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "select__first_resample",
                                    "id" : "{id}"
                                    }
                                ),
                            *88 -> PipeStageArguments(
                                constraints = [ { "id" : "Russell.*close:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "select__first_resample",
                                    "id" : "{id}"
                                    }
                                ),
                            *89 -> PipeStageArguments(
                                constraints = [ { "id" : "USDollar:cash:close:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "select__first_resample",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *91 -> ResamplePipeStage(
                        stage_name = "LSEUSIndex::cleanData()",
                        instages = [ *90; ],
                        dominate_inputs_by = "id",
                        intersect_calendars = 1,
                        strip_all_missings = 1
                        ),
                    *93 -> SelectPipeStage(
                        stage_name = "russel1000-growth-index",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *92 -> PipeStageArguments(
                                constraints = [ { "id" : "russell1000growthindex" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "russel1000-growth-index",
                                    "id" : "russel1000-growth-index"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *95 -> SelectPipeStage(
                        stage_name = "russel1000-value-index",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *94 -> PipeStageArguments(
                                constraints = [ { "id" : "russell1000valueindex" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "russel1000-value-index",
                                    "id" : "russel1000-value-index"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *97 -> SelectPipeStage(
                        stage_name = "russel2000-growth-index",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *96 -> PipeStageArguments(
                                constraints = [ { "id" : "russell2000growthindex" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "russel2000-growth-index",
                                    "id" : "russel2000-growth-index"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *99 -> SelectPipeStage(
                        stage_name = "russel2000-value-index",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *98 -> PipeStageArguments(
                                constraints = [ { "id" : "russell2000valueindex" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "russel2000-value-index",
                                    "id" : "russel2000-value-index"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *102 -> GenericPipeStage(
                        stage_name = "ts_return1__russel1000-growth-index",
                        instages = [ *93; ],
                        dominate_inputs_by = "",
                        args = [
                            *100 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__russel1000-growth-index",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *101 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *105 -> GenericPipeStage(
                        stage_name = "ts_return1__russel1000-value-index",
                        instages = [ *95; ],
                        dominate_inputs_by = "",
                        args = [
                            *103 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__russel1000-value-index",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *104 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *108 -> GenericPipeStage(
                        stage_name = "ts_return1__russel2000-growth-index",
                        instages = [ *97; ],
                        dominate_inputs_by = "",
                        args = [
                            *106 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__russel2000-growth-index",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *107 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *111 -> GenericPipeStage(
                        stage_name = "ts_return1__russel2000-value-index",
                        instages = [ *99; ],
                        dominate_inputs_by = "",
                        args = [
                            *109 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__russel2000-value-index",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *110 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *114 -> GenericPipeStage(
                        stage_name = "affine0.34Xp0.00__ts_return1__russel1000-growth-index",
                        instages = [ *102; ],
                        dominate_inputs_by = "",
                        args = [
                            *112 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine0.34Xp0.00__ts_return1__russel1000-growth-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *113 -> AffineTransformTimeSeries(
                            a = [ 0.3375 ],
                            b = [ 0 ]
                            )
                        ),
                    *117 -> GenericPipeStage(
                        stage_name = "affine0.34Xp0.00__ts_return1__russel1000-value-index",
                        instages = [ *105; ],
                        dominate_inputs_by = "",
                        args = [
                            *115 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine0.34Xp0.00__ts_return1__russel1000-value-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *116 -> AffineTransformTimeSeries(
                            a = [ 0.3375 ],
                            b = [ 0 ]
                            )
                        ),
                    *120 -> GenericPipeStage(
                        stage_name = "affine0.16Xp0.00__ts_return1__russel2000-growth-index",
                        instages = [ *108; ],
                        dominate_inputs_by = "",
                        args = [
                            *118 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine0.16Xp0.00__ts_return1__russel2000-growth-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *119 -> AffineTransformTimeSeries(
                            a = [ 0.1625 ],
                            b = [ 0 ]
                            )
                        ),
                    *123 -> GenericPipeStage(
                        stage_name = "affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *111; ],
                        dominate_inputs_by = "",
                        args = [
                            *121 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *122 -> AffineTransformTimeSeries(
                            a = [ 0.1625 ],
                            b = [ 0 ]
                            )
                        ),
                    *126 -> GenericPipeStage(
                        stage_name = "add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index",
                        instages = [
                            *114;,
                            *117;
                            ],
                        dominate_inputs_by = "",
                        args = [
                            *124 -> PipeStageArguments(
                                constraints = [
                                    {
                                        "stage_name" : "affine0.34Xp0.00__ts_return1__russel1000-growth-index",
                                        "id" : ".*"
                                        },
                                    {
                                        "stage_name" : "affine0.34Xp0.00__ts_return1__russel1000-value-index",
                                        "id" : ".*"
                                        }
                                    ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index",
                                    "id" : "{id}:add:affine0.34Xp0.00__ts_return1__russel1000-value-index"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *125 -> AddTimeSeries( )
                        ),
                    *129 -> GenericPipeStage(
                        stage_name = "add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [
                            *120;,
                            *123;
                            ],
                        dominate_inputs_by = "",
                        args = [
                            *127 -> PipeStageArguments(
                                constraints = [
                                    {
                                        "stage_name" : "affine0.16Xp0.00__ts_return1__russel2000-growth-index",
                                        "id" : ".*"
                                        },
                                    {
                                        "stage_name" : "affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                        "id" : ".*"
                                        }
                                    ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}:add:affine0.16Xp0.00__ts_return1__russel2000-value-index"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *128 -> AddTimeSeries( )
                        ),
                    *132 -> GenericPipeStage(
                        stage_name = "add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [
                            *126;,
                            *129;
                            ],
                        dominate_inputs_by = "",
                        args = [
                            *130 -> PipeStageArguments(
                                constraints = [
                                    {
                                        "stage_name" : "add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index",
                                        "id" : ".*"
                                        },
                                    {
                                        "stage_name" : "add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                        "id" : ".*"
                                        }
                                    ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}:add:add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *131 -> AddTimeSeries( )
                        ),
                    *135 -> GenericPipeStage(
                        stage_name = "affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *132; ],
                        dominate_inputs_by = "",
                        args = [
                            *133 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *134 -> AffineTransformTimeSeries(
                            a = [ 1 ],
                            b = [ 1.0 ]
                            )
                        ),
                    *138 -> GenericPipeStage(
                        stage_name = "cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *135; ],
                        dominate_inputs_by = "",
                        args = [
                            *136 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}:cumprod"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *137 -> CumProdTimeSeries( )
                        ),
                    *141 -> GenericPipeStage(
                        stage_name = "set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *138; ],
                        dominate_inputs_by = "",
                        args = [
                            *139 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "lse-us-index"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *140 -> IdentityTimeSeries( )
                        ),
                    *144 -> GenericPipeStage(
                        stage_name = "ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *141; ],
                        dominate_inputs_by = "",
                        args = [
                            *142 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *143 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *147 -> GenericPipeStage(
                        stage_name = "affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *141; ],
                        dominate_inputs_by = "",
                        args = [
                            *145 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *146 -> AffineTransformTimeSeries(
                            a = [ -1 ],
                            b = [ 0 ]
                            )
                        ),
                    *150 -> GenericPipeStage(
                        stage_name = "set_id__affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *147; ],
                        dominate_inputs_by = "",
                        args = [
                            *148 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "set_id__affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "short-{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *149 -> IdentityTimeSeries( )
                        ),
                    *153 -> GenericPipeStage(
                        stage_name = "affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *144; ],
                        dominate_inputs_by = "",
                        args = [
                            *151 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *152 -> AffineTransformTimeSeries(
                            a = [ -1 ],
                            b = [ 0 ]
                            )
                        ),
                    *156 -> GenericPipeStage(
                        stage_name = "set_id__affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                        instages = [ *153; ],
                        dominate_inputs_by = "",
                        args = [
                            *154 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "set_id__affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index",
                                    "id" : "short-{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *155 -> IdentityTimeSeries( )
                        ),
                    *158 -> SelectPipeStage(
                        stage_name = "pseudo-prices",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *157 -> PipeStageArguments(
                                constraints = [ { "id" : ".*:pseudo:close:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "pseudo-prices",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *161 -> GenericPipeStage(
                        stage_name = "ts_return1__pseudo-prices",
                        instages = [ *158; ],
                        dominate_inputs_by = "",
                        args = [
                            *159 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "ts_return1__pseudo-prices",
                                    "id" : "{id}:ts_return1"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *160 -> ReturnTimeSeries(
                            horizon = 1,
                            return_type = "relative"
                            )
                        ),
                    *163 -> SelectPipeStage(
                        stage_name = "FederalFunds:cash:close:level",
                        instages = [ *91; ],
                        dominate_inputs_by = "",
                        args = [
                            *162 -> PipeStageArguments(
                                constraints = [ { "id" : "FederalFunds:cash:close:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "FederalFunds:cash:close:level",
                                    "id" : "FederalFunds:cash:close:level"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        ),
                    *166 -> GenericPipeStage(
                        stage_name = "affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [ *163; ],
                        dominate_inputs_by = "",
                        args = [
                            *164 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine0.00Xp0.00__FederalFunds:cash:close:level",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *165 -> AffineTransformTimeSeries(
                            a = [ 2.77777777778e-05 ],
                            b = [ 0 ]
                            )
                        ),
                    *169 -> GenericPipeStage(
                        stage_name = "affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [ *166; ],
                        dominate_inputs_by = "",
                        args = [
                            *167 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *168 -> AffineTransformTimeSeries(
                            a = [ 1 ],
                            b = [ 1.0 ]
                            )
                        ),
                    *172 -> GenericPipeStage(
                        stage_name = "cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [ *169; ],
                        dominate_inputs_by = "",
                        args = [
                            *170 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                                    "id" : "{id}:cumprod"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *171 -> CumProdTimeSeries( )
                        ),
                    *175 -> GenericPipeStage(
                        stage_name = "set_id__cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [ *172; ],
                        dominate_inputs_by = "",
                        args = [
                            *173 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "set_id__cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                                    "id" : "FederalFunds:cash:close:level:index"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *174 -> IdentityTimeSeries( )
                        ),
                    *176 -> ResamplePipeStage(
                        stage_name = "tight_resample__LSEUSIndex::cleanData()_&&_set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_first_resample_&&_ts_return1__pseudo-prices_&&_ts_return1__pseudo-prices_&&_set_id__cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [
                            *91;,
                            *141;,
                            *150;,
                            *144;,
                            *156;,
                            *85;,
                            *161;,
                            *161;,
                            *175;
                            ],
                        dominate_inputs_by = "id",
                        intersect_calendars = 1,
                        lower_fdate = 1000101,
                        strip_all_missings = 0,
                        unite_calendars = 0,
                        upper_fdate = 1041231
                        ),
                    *179 -> GenericPipeStage(
                        stage_name = "Last day of month",
                        instages = [ *176; ],
                        dominate_inputs_by = "",
                        args = [
                            *177 -> PipeStageArguments(
                                constraints = [ { "id" : "Russell1000:open:level" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "Last day of month",
                                    "id" : "last_day_of_month"
                                    }
                                )
                            ],
                        copy_inputs = 0,
                        model = *178 -> CalendarIndicatorTimeSeries( day_of_month_indicator = [ -1 ] )
                        ),
                    *181 -> SelectPipeStage(
                        stage_name = "merge__Last day of month_&&_tight_resample__LSEUSIndex::cleanData()_&&_set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_first_resample_&&_ts_return1__pseudo-prices_&&_ts_return1__pseudo-prices_&&_set_id__cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                        instages = [
                            *179;,
                            *176;
                            ],
                        dominate_inputs_by = "id",
                        args = [
                            *180 -> PipeStageArguments(
                                constraints = [ { "id" : ".*" } ],
                                params = { },
                                output_tags = {
                                    "stage_name" : "merge__Last day of month_&&_tight_resample__LSEUSIndex::cleanData()_&&_set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_set_id__affine-1.00Xp0.00__ts_return1__set_id__cumprod__affine1.00Xp1.00__add__add__affine0.34Xp0.00__ts_return1__russel1000-growth-index_&&_affine0.34Xp0.00__ts_return1__russel1000-value-index_&&_add__affine0.16Xp0.00__ts_return1__russel2000-growth-index_&&_affine0.16Xp0.00__ts_return1__russel2000-value-index_&&_first_resample_&&_ts_return1__pseudo-prices_&&_ts_return1__pseudo-prices_&&_set_id__cumprod__affine1.00Xp1.00__affine0.00Xp0.00__FederalFunds:cash:close:level",
                                    "id" : "{id}"
                                    }
                                )
                            ],
                        copy_inputs = 0
                        )
                    ],
                outputs = [ *181; ]
                ),
            dict_ids = [ "id" ],
            sort_by = "id"
            ),
        accessory_learners = [ *41; ]
        )
"""
from apstat.finance.pyplearn import *

#
#  Preprocessing
#
from apstat.finance.release.lse_200509.LSE_preproc import Preproc, getPreproc
Preproc.return_horizons = [ 1 ]
pipeline, index_name, return_families, target_family = getPreproc( )

#
#  Environment and options
#

class Model( plargs_binder ):
    kinds           = [ 'UnitedStates:LSE13' ]
    linreg_wd       = 1e-05       # LinearRegressor's Weight decay
    max_train_size  = 120         # Window selected by Marc Boucher (DGIA)
    cash_level      = 100e06 # 73e06# 
    
class Trading( plargs_binder ):
    pf_thresh       = 0.1         # ThresholdDecision

#
# AssetManager specific settings
#
from apstat.finance.release.lse_200509.LSEAssetManager import LSEAssetManager
asset_manager   = LSEAssetManager( Model.kinds, index_name, return_families, target_family )

test_start_time = Model.max_train_size + 1 # Providing enough observations to
                                           # the LinearRegressor

#
#  Model: Advisor and Trader
#

linear_model = pl.EmbeddedPLearnerAdvisor(
    learner         = pl.LinearRegressor( weight_decay = Model.linreg_wd,
                                          output_learned_weights = 1,
                                          include_bias = 0 ),
    horizon         = 0,
    default_class   = 'asset',
    input_sefams    = [ ( '', sefam_name )
                        for (sefam_name, sefam) in return_families ],
    target_sefams   = [ ( 'target_returns', 'any' ) ],

    train_on_test_input = 1,
    
    dataset_wrapper = pl.DisregardRowsVMatrix( maximum_length = Model.max_train_size )
    )

#
#  Trader
#

#
#  Modifying the default behaviour!
#
TrCostExecutionPlugin.global_proportional_cost = 0.001

def LSETrader( advisor, amng, noc_decision ):
    ## scale_dec   = ScaleDecision( scale_min_fraction = 0.9,
    ##                              scale_max_fraction = 1.5 )

    integr_dec  = IntegralizeDecision( truncate = 0 )

    thresh_dec  = ThresholdDecision( portfolio_relative_threshold = 0.02,
                                     assetwise_absolute_threshold = 5
                                     )

    dec_plugins = DecisionPlugins(
        noc_decision                 = noc_decision,
        scale_to_volatility_decision = None,
        scale_decision               = None,# scale_dec,#
        max_exposure_decision        = None,
        integralize_decision         = integr_dec,
        threshold_decision           = thresh_dec
        )

    exec_plugins = ExecutionPlugins(interest_on_cash_execution = None)

    perf_plugins = PerfCostPlugins(
        LSE_US_Index    = PerfPortfolioReturn( name        = "LSE_US_Index_daily",
                                               bench_index = index_name,
                                               bench_only  = 1
                                               ),
        vs_LSE_US_Index = PerfPortfolioReturn( name        = "vs_LSE_US_Index_daily", 
                                               bench_index = index_name,
                                               bench_only  = 0
                                               )
        )

    return ApTrader( advisor, amng,
                     initial_margin           = Model.cash_level,
                     decision_plugins         = dec_plugins,
                     execution_plugins        = exec_plugins,
                     test_performance_plugins = perf_plugins
                     )

#
#  Make the exposure of the short side by relative to the margin of the long side.
#

from apstat.finance.release.lse_200509.LongSideProxyTrader import LongSideProxyTrader
long_side_trader = LongSideProxyTrader( index_name, Model.cash_level )

exo_noc_dec      = ExogenousExposureNOCDecision(
    global_leverage  = -1,
    exogenous_trader = long_side_trader
    )

trader           = LSETrader( linear_model, asset_manager, exo_noc_dec )


#
#  Top-level: SequentialValidation
#
sequential_validation = PipelineSequentialValidation(
    learner                  = trader,
    accessory_learners       = [ long_side_trader ],
    pipeline                 = pipeline,
    expdir                   = plargs.expdir,
    init_train_size          = test_start_time,
    save_test_outputs        = 1,
    )

from apstat.finance.pyplearn.SequentialValidation import stats_cprod
sequential_validation.statnames.extend(
    stats_cprod(\
        [ 'E', 'STDDEV' ],
        ## 
        [ 'LSE_US_Index_daily_relative_return',
          'LSE_US_Index_daily_absolute_return',
          'vs_LSE_US_Index_daily_absolute_return'
          ]
        ) +     
    stats_cprod(\
        [ 'E', 'STDDEV', 'SHARPERATIO', 'MIN', 'MAX', 'SKEW', 'KURT' ],
        ## 
        [ 'vs_LSE_US_Index_daily_relative_return' ]
        )    
    )    

def main( ):
    return sequential_validation

