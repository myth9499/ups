SRCHOME=/item/ups/src
subsystem:
	@echo	"@@@@@@@@开始编译统一支付平台V1.0@@@@@@@@@@"
	@echo	"******************************************"
	@echo	"*************开始编译base模块*************"
	@cd	$(SRCHOME)/base	&& make
	@echo	"*************开始编译hash模块*************"
	@cd	$(SRCHOME)/hash	&& make
	@echo	"*************开始编译unpack模块*************"
	@cd	$(SRCHOME)/unpack	&& make
	@echo	"*************开始编译commmsg模块*************"
	@cd	$(SRCHOME)/commmsg	&& make
	@echo	"*************开始编译shm模块*************"
	@cd	$(SRCHOME)/shm	&& make
	@echo	"*************开始编译server模块*************"
	@cd	$(SRCHOME)/server	&& make
	@echo	"*************开始编译monitor模块*************"
	@cd	$(SRCHOME)/monitor	&& make
	@echo	"*************开始编译cfg模块*************"
	@cd	$(SRCHOME)/cfg	&& make
	@echo	"*************开始编译channel模块*************"
	@cd	$(SRCHOME)/channel	&& make
	@echo	"@@@@@@@@@@@编译统一支付平台V1.0结束@@@@@@@@"
clean:
	@cd /item/ups/src && find . -name "*.o" -print|xargs rm -rf 
