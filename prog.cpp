#include <stdio.h>
//#include "Wrapper.h"
#include "InstrumentData.h"
int main()
{
	std::auto_ptr<InstrumentData> data( new InstrumentData() );
	//data.process();
	//data.populate();
	
	//work for issue 1.
	data.get()->process();
	
  return 0;
}

DECLARE @intDate DateTime
SET @intDate = '2010-10-04'

WHILE (@intDate <= '2014-10-31' )
BEGIN
    PRINT @intDate
   
    DECLARE @date datetime
       SET @date = convert( datetime, @intDate , 101)

       insert into suspendAccount
       select
        @date ,
        aid ,
        accno ,
        fundMove ,
        stockMove ,
        tradedone
       from (
       select
        clm .aid,
        clm .accno,
        isnull(fms .fundMove, 'N') as fundMove ,
        isnull(sms .stockMove, 'N') as stockMove ,
        isnull(td .trade, 'N') as tradeDone
       from ( 
        select aid, accno
        from G2BS_UAT. G2SB_UAT4.dbo .client_master cm
        where exists (
        select 1 from G2BS_UAT.G2SB_UAT4 .dbo. client_master_s cms
        where cms. date_open <= @date
        and cms. aid = cm .aid
        ) and exists (
        select * from G2BS_UAT .G2SB_UAT4. dbo.suspense_master sm
        where rtrim (sm. sreason) is not null
        and sm. spid = cm .spid
        and rtrim (sm. sreason) in (
        'Buy Only',
       'RETRUN MAIL',
       'RETURN MAIL',
       'RETUENED MAIL',
       'RRETUEN MAIL',
       'Sell Only',
       'Suspend buy & sell',
       'Suspend trade only',
       'SUSPEND TRADING'
        )
       )) clm
       left join (
             select aid, 'Y' as fundMove from (
              select
                  aid
              from
                  G2BS_UAT .G2SB_UAT4. dbo.fund_move_client
              where
                   DATEADD(dd , 0, DATEDIFF( dd, 0 , date )) = @date
                    and purpose in ('0' , '2' , '4' , '9' )
             UNION
              select
                  aid
              from
                  G2BS_UAT .G2SB_UAT4. dbo.histcl_fund
              where
                   DATEADD(dd , 0, DATEDIFF( dd, 0 , date )) = @date
                    and purpose in ('0' , '2' , '4' , '9' )
             ) fm
       ) fms
       ON
        fms .aid = clm.aid     
       LEFT JOIN (
        select
            aid ,
             'Y' as stockMove
        FROM (
             select
              aid
             from
              G2BS_UAT .G2SB_UAT4. dbo.stk_move_client
             where
              purpose not in ('1' , '3' , 'C' )
                   --and vdate = @date
                   and DATEADD (dd, 0, DATEDIFF(dd , 0, date)) = @date
                   and aid <> '-1'
             union
             select
              aid
             from
              G2BS_UAT .G2SB_UAT4. dbo.hist_stk_move_client
             where
              purpose not in ('1' , '3' , 'C' )
                   --and vdate = @date
                   and DATEADD (dd, 0, DATEDIFF(dd , 0, date)) = @date
                   and aid <> '-1'
        ) sm
       ) sms
       ON
        sms .aid = clm.aid 
       LEFT JOIN (
        SELECT
            aid ,
             'Y' as trade
        FROM (   
             select
              aid      
             from
              G2BS_UAT .G2SB_UAT4. dbo.view_ctrade_namt_order
             where
              tdate = @date
             union
             select
              aid
             from
              G2BS_UAT .G2SB_UAT4. dbo.view_hs_ctrade_namt_order
             where
              tdate = @date
        ) trade
       ) td
       ON
        td .aid = clm.aid 
       ) temp where (fundMove = 'Y' or stockMove = 'Y' or tradeDone = 'Y')
         
   
    SET @intDate = DATEADD( day,1 , @intDate)
END

//----------------
declare @tradeDate nvarchar (10)
set @tradeDate = '2011-04-21'

SELECT
      masterTable .acc_no as accno,
      masterTable .spid as spid,
      masterTable .name_1 as name,
      masterTable .suspend_code as suspend_reason,
       --ISNULL(fundMovement.exist_fund,'N') AS fund_movement,
       --ISNULL(stockMovement.exist_stock,'N') AS stock_movement,
       ISNULL(tradeDetail .exist_trade, 'N') AS trade_done,
       REPLACE(
             REPLACE(
                   CONVERT(VARCHAR (30), ISNULL(masterTable .last_tran_date, '')
                   , 103)
             , '-' ,''), '01/01/1900', '') AS tran_date
 FROM
 (
       SELECT
            clm .accno AS acc_no,
            clm .spid as spid,
            clms .aid  AS aid,
            clm .name_1 AS name_1,
            clms .date_last AS last_tran_date,
             RTRIM(ISNULL (sus_m .sreason, '')) AS suspend_code
       FROM
            G2BS_UAT .G2SB_UAT4. dbo.client_master AS clm
             LEFT JOIN
                  G2BS_UAT .G2SB_UAT4. dbo.client_master_s AS clms
             ON
                  clms .aid = clm.aid
            
             LEFT JOIN
                  G2BS_UAT .G2SB_UAT4. dbo.suspense_master AS sus_m
             ON
                  clm .spid = sus_m.spid
            
             WHERE /*EXISTS
                  (
                        SELECT 1
                        FROM
                              ReportScheduleParam AS rsp
                        WHERE
                              rsp.ReportSchPGroup = 'SuspendCode'
                              AND sus_m.sreason COLLATE Chinese_Taiwan_Stroke_CI_AS = rsp.CharValue
                  )
            AND*/ clms.date_open <= @tradeDate
 ) AS masterTable
 LEFT JOIN
 (
   SELECT
      td .aid, 'Y' AS exist_trade
   FROM
   (
        SELECT
             DISTINCT cno. aid
        FROM
            G2BS_UAT .G2SB_UAT4. dbo.view_ctrade_namt_order AS cno
        WHERE
            tdate = @tradeDate
        GROUP BY cno.aid
        UNION
        SELECT
             DISTINCT hcno. aid
        FROM
            G2BS_UAT .G2SB_UAT4. dbo.view_hs_ctrade_namt_order AS hcno
        WHERE
            tdate = @tradeDate
        GROUP BY hcno.aid
   ) AS td
  
  
 ) AS tradeDetail
 ON
 tradeDetail.aid = masterTable. aid
 where
      tradeDetail .exist_trade = 'Y'
       and masterTable. spid > 0 
