set(TR_SOURCE
  main.c
  check_osu_file.c		check_osu_file.h
  tr_mods.c			tr_mods.h
  print.c			print.h
  bpm.h
  taiko_ranking_map.c           taiko_ranking_map.h
  taiko_ranking_object.c        taiko_ranking_object.h
  taiko_ranking_score.c	        taiko_ranking_score.h
  treatment.c			treatment.h
  tr_sort.c			tr_sort.h
  tr_db.c			tr_db.h
  tr_gts.c			tr_gts.h
  config.c			config.c
  options.c 			options.h
  cst_yaml.c			cst_yaml.h
  vector.c			vector.h
  linear_fun.c			linear_fun.h
  spacing_count.c		spacing_count.h
  freq_counter.c		freq_counter.h
  compute_stars.c		compute_stars.h
  density.c			density.h
  reading.c			reading.h
  pattern.c			pattern.h
  accuracy.c			accuracy.h
  final_star.c			final_star.h
  )

if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GTS_C_FLAGS}")

add_executable(taiko_ranking ${TR_SOURCE})
target_link_libraries(taiko_ranking
  osux m mysqlclient ${CMAKE_THREAD_LIBS_INIT} ${GTS_LIBRARIES})

add_sanitizers(taiko_ranking)

install( 
  TARGETS taiko_ranking
  RUNTIME DESTINATION bin
  )

install(
  DIRECTORY yaml
  DESTINATION etc/osux
  )
